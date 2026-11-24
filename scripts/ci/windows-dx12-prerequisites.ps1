[CmdletBinding()]
param(
  [Parameter(Mandatory)][ValidateSet('hosted-common','desktop')][string]$Lane,
  [Parameter(Mandatory)][string]$OutputRoot
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$output = [IO.Path]::GetFullPath($OutputRoot)
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
if ($output -eq $repo -or $output.StartsWith($repo.TrimEnd('\') + '\', [StringComparison]::OrdinalIgnoreCase)) {
  throw 'CI evidence must be outside the repository'
}
New-Item -ItemType Directory -Path $output -Force | Out-Null
$reportPath = Join-Path $output 'prerequisites.json'
if (Test-Path -LiteralPath $reportPath) { throw 'Refusing to overwrite first preflight evidence' }
$report = [ordered]@{ lane=$Lane; passed=$false; native_gpu_probe='not-yet-run'; failures=@() }
try {
  if ($env:OS -ne 'Windows_NT' -or
      [Runtime.InteropServices.RuntimeInformation]::ProcessArchitecture -ne 'X64') { throw 'Windows x64 process required' }
  $os = Get-CimInstance Win32_OperatingSystem
  $version = Get-ItemProperty 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion'
  $report.os = @{ caption=$os.Caption; product_type=[int]$os.ProductType; build=[int]$version.CurrentBuildNumber;
                 ubr=[int]$version.UBR; architecture=$os.OSArchitecture }
  $report.powershell = $PSVersionTable.PSVersion.ToString()
  $report.image = @{ os=$env:ImageOS; version=$env:ImageVersion; runner_environment=$env:RUNNER_ENVIRONMENT;
                    desktop_image_id=$env:CGPUI_DX12_DESKTOP_IMAGE_ID }
  $report.session_id = [Diagnostics.Process]::GetCurrentProcess().SessionId
  $report.user_interactive = [Environment]::UserInteractive
  Add-Type -Path (Join-Path $PSScriptRoot 'windows_dx12_desktop.cs')
  $report.desktop = [CgpuiDx12DesktopFacts]::Capture()
  $capability = Get-WindowsCapability -Online -Name 'Tools.Graphics.DirectX~~~~0.0.1.0'
  $report.graphics_tools = @{ name=$capability.Name; state=$capability.State.ToString() }
  if ($capability.State -ne 'Installed') { throw 'Graphics Tools must be pre-provisioned; no install/disable fallback' }
  $report.dlls = @{}
  foreach ($name in @('d3d12.dll','D3D12Core.dll','d3d12SDKLayers.dll','dxgi.dll','dwrite.dll')) {
    $path = Join-Path $env:SystemRoot "System32\$name"
    $report.dlls[$name] = if (Test-Path -LiteralPath $path) { (Get-Item -LiteralPath $path).VersionInfo.FileVersion } else { $null }
  }
  $report.fonts = @((Get-ItemProperty 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts').PSObject.Properties |
    Where-Object { $_.Name -match '^(Segoe UI|Microsoft YaHei|Microsoft JhengHei|SimSun|Noto Sans CJK)' } |
    Select-Object Name,Value)
  $mcpp = if ($env:MCPP) { $env:MCPP } else { (Get-Command mcpp -ErrorAction Stop).Source }
  $report.mcpp = @{ path=$mcpp; version=(& $mcpp --version | Out-String).Trim();
                    sha256=(Get-FileHash -LiteralPath $mcpp -Algorithm SHA256).Hash }
  if ($LASTEXITCODE -ne 0) { throw 'mcpp version command failed' }
  $report.vulkan_sdk = $env:VULKAN_SDK
  foreach ($relative in @('Include\vulkan\vulkan.h','Lib\vulkan-1.lib')) {
    if (-not $env:VULKAN_SDK -or -not (Test-Path -LiteralPath (Join-Path $env:VULKAN_SDK $relative))) {
      throw "Required build-time Vulkan SDK entry absent: $relative"
    }
  }
  $report.dependency_archives = @()
  if ($env:RUNNER_TEMP) {
    foreach ($name in @('mcpp-windows-x86_64.zip','vulkansdk-windows-X64-1.4.341.1.exe')) {
      $path = Join-Path $env:RUNNER_TEMP $name
      if (Test-Path -LiteralPath $path) {
        $report.dependency_archives += @{ name=$name; sha256=(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash }
      }
    }
  }
  if ($Lane -eq 'desktop') {
    if ($os.ProductType -ne 1 -or $report.os.build -lt 26100 -or $report.os.ubr -le 0) {
      throw 'Full desktop lane requires Windows client build26100+ with recorded UBR'
    }
    if (-not $env:CGPUI_DX12_DESKTOP_IMAGE_ID) { throw 'Provisioned disposable desktop image identity required' }
    $desktop = $report.desktop
    if (-not $report.user_interactive -or $report.session_id -le 0 -or
        $report.session_id -ne $desktop.ActiveConsoleSession -or $desktop.RemoteSession -or
        $desktop.InputDesktop -ne 'Default' -or $desktop.InputDesktopError -ne 0 -or
        $desktop.CompositionResult -ne 0 -or -not $desktop.CompositionEnabled -or
        -not $desktop.WorkAreaAvailable -or $desktop.WorkWidth -lt 500 -or $desktop.WorkHeight -lt 380) {
      throw 'Full native suite requires unlocked local console desktop, DWM and adequate work area'
    }
    $vulkanInfo = Join-Path $env:VULKAN_SDK 'Bin\vulkaninfo.exe'
    $vulkanLog = Join-Path $output 'vulkan-preflight.log'
    & $vulkanInfo --summary *> $vulkanLog
    $report.vulkan_preflight_exit = $LASTEXITCODE
    if ($LASTEXITCODE -ne 0) { throw 'Full desktop lane requires a real healthy Vulkan ICD as well as WARP' }
  }
  # DLL/capability checks do not prove usable WARP/SM6/InfoQueue/GBV or text glyphs.
  # The subsequent exact identity+13 scene execution is the native positive gate.
  $report.passed = $true
} catch {
  $report.failures += $_.Exception.Message
} finally {
  $report | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $reportPath -Encoding utf8
}
if (-not $report.passed) { throw ($report.failures -join '; ') }
