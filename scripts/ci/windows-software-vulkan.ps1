# Only for the ephemeral hosted runner. Local runs use their installed GPU.
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

# The x64 Chrome installed on windows-latest ships SwiftShader, including
# VK_KHR_win32_surface. Point the Vulkan loader at that exact driver rather
# than requiring the runner to expose a physical GPU.
$chrome = Join-Path $env:ProgramFiles 'Google\Chrome\Application'
$versions = Get-ChildItem -LiteralPath $chrome -Directory |
  Where-Object { $_.Name -match '^\d+\.\d+\.\d+\.\d+$' } |
  Sort-Object { [version]$_.Name } -Descending
$manifest = $null
foreach ($version in $versions) {
  $file = Join-Path $version.FullName 'vk_swiftshader_icd.json'
  if (!(Test-Path -LiteralPath $file -PathType Leaf)) { continue }
  $icd = (Get-Content -Raw -LiteralPath $file | ConvertFrom-Json).ICD
  if (Test-Path -LiteralPath (Join-Path $version.FullName $icd.library_path) -PathType Leaf) {
    $manifest = $file
    break
  }
}
if (!$manifest) { throw "No x64 SwiftShader Vulkan driver found in $chrome" }

$env:VK_DRIVER_FILES = $manifest
"VK_DRIVER_FILES=$manifest" >> $env:GITHUB_ENV
# Elevated loaders may ignore environment-based discovery. Use the normal
# Windows driver registration in that case (the hosted VM is disposable).
$identity = [Security.Principal.WindowsIdentity]::GetCurrent()
if (([Security.Principal.WindowsPrincipal]::new($identity)).IsInRole(
    [Security.Principal.WindowsBuiltInRole]::Administrator)) {
  $key = 'HKLM:\SOFTWARE\Khronos\Vulkan\Drivers'
  if (!(Test-Path $key)) { New-Item -Path $key -Force | Out-Null }
  New-ItemProperty -Path $key -Name $manifest -PropertyType DWord -Value 0 -Force | Out-Null
}
$info = Get-ChildItem -LiteralPath (Join-Path $env:VULKAN_SDK 'Bin') -Filter 'vulkaninfo*.exe' |
  Select-Object -First 1
if (!$info) { throw 'The installed Vulkan SDK has no vulkaninfo' }
$summary = & $info.FullName --summary 2>&1
$exit = $LASTEXITCODE
$summary | Out-Host
if ($exit -ne 0 -or ($summary -join "`n") -notmatch 'SwiftShader') {
  throw "vulkaninfo did not enumerate SwiftShader (exit $exit, driver $manifest)"
}
