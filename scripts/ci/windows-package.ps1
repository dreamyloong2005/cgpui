[CmdletBinding()]
param(
  [Parameter(Mandatory = $true)]
  [ValidateSet("Debug", "Release")]
  [string]$Mode,
  [string]$OutputRoot = ""
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

if ($env:OS -ne "Windows_NT") {
  throw "windows-package.ps1 requires Windows"
}

$repoRoot = [System.IO.Path]::GetFullPath(
  (Join-Path $PSScriptRoot "..\..")
)
$modeName = $Mode.ToLowerInvariant()
if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
  $OutputRoot = Join-Path $repoRoot "build\phase-g-ci\windows-$modeName"
} elseif (-not [System.IO.Path]::IsPathRooted($OutputRoot)) {
  $OutputRoot = Join-Path $repoRoot $OutputRoot
}
$outputRoot = [System.IO.Path]::GetFullPath($OutputRoot)
$repoPrefix = $repoRoot.TrimEnd('\', '/') +
  [System.IO.Path]::DirectorySeparatorChar
if ($outputRoot -eq $repoRoot -or
    -not $outputRoot.StartsWith(
      $repoPrefix,
      [System.StringComparison]::OrdinalIgnoreCase
    )) {
  throw "output root must stay inside the repository: $outputRoot"
}

if (Test-Path -LiteralPath $outputRoot) {
  Remove-Item -LiteralPath $outputRoot -Recurse -Force
}
$buildRoot = Join-Path $outputRoot "build-root"
$configRoot = Join-Path $outputRoot "config"
$packageRoot = Join-Path $outputRoot "package"
$libRoot = Join-Path $packageRoot "lib"
$binRoot = Join-Path $packageRoot "bin"
$includeRoot = Join-Path $packageRoot "include"
New-Item -ItemType Directory -Force -Path @(
  $buildRoot, $configRoot, $packageRoot, $libRoot, $binRoot, $includeRoot
) | Out-Null

. (Join-Path $PSScriptRoot "windows-dependencies.ps1")
Set-CgpuiWindowsDependencyEnvironment
$env:XMAKE_CONFIGDIR = $configRoot

Push-Location $repoRoot
try {
  & xmake f -P $repoRoot -y -c -m $modeName --ccache=n -o $buildRoot
  if ($LASTEXITCODE -ne 0) {
    throw "xmake f failed with exit code $LASTEXITCODE"
  }
  & xmake build -P $repoRoot -j 1 cgpui_app
  if ($LASTEXITCODE -ne 0) {
    throw "xmake build cgpui_app failed with exit code $LASTEXITCODE"
  }
  & xmake build -P $repoRoot -j 1 hello_window
  if ($LASTEXITCODE -ne 0) {
    throw "xmake build hello_window failed with exit code $LASTEXITCODE"
  }
} finally {
  Pop-Location
}

$binaryRoot = Join-Path $buildRoot "windows\x64\$modeName"
$libraries = @(
  "cgpui_core.lib",
  "cgpui_platform.lib",
  "cgpui_platform_win32.lib",
  "cgpui_renderer.lib",
  "cgpui_renderer_vulkan.lib",
  "cgpui_ui.lib",
  "cgpui_app.lib"
)
foreach ($library in $libraries) {
  $source = Join-Path $binaryRoot $library
  if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
    throw "missing Windows $Mode library: $source"
  }
  Copy-Item -LiteralPath $source -Destination $libRoot
}

$demo = Join-Path $binaryRoot "hello_window.exe"
if (-not (Test-Path -LiteralPath $demo -PathType Leaf)) {
  throw "missing Windows $Mode demo: $demo"
}
Copy-Item -LiteralPath $demo -Destination $binRoot
Copy-Item -LiteralPath (Join-Path $repoRoot "include\cgpui") `
  -Destination $includeRoot -Recurse
Copy-Item -LiteralPath (Join-Path $repoRoot "README.md") `
  -Destination $packageRoot

$manifest = [ordered]@{
  schema_version = 1
  package = "cgpui-windows-$modeName"
  platform = "windows"
  architecture = "x64"
  mode = $modeName
  header_root = "include/cgpui"
  libraries = @($libraries | ForEach-Object { "lib/$_" })
  executables = @("bin/hello_window.exe")
  documents = @("README.md")
}
$manifestPath = Join-Path $packageRoot "manifest.json"
$manifest | ConvertTo-Json -Depth 4 | Set-Content `
  -LiteralPath $manifestPath -Encoding utf8

$parsedManifest = Get-Content -Raw -LiteralPath $manifestPath |
  ConvertFrom-Json
if ($parsedManifest.schema_version -ne 1 -or
    $parsedManifest.package -ne "cgpui-windows-$modeName" -or
    $parsedManifest.platform -ne "windows" -or
    $parsedManifest.architecture -ne "x64" -or
    $parsedManifest.mode -ne $modeName -or
    @($parsedManifest.libraries).Count -ne $libraries.Count -or
    @($parsedManifest.executables).Count -ne 1) {
  throw "Windows $Mode manifest validation failed"
}

$requiredPackagePaths = @(
  "include/cgpui",
  "bin/hello_window.exe",
  "README.md",
  "manifest.json"
) + @($libraries | ForEach-Object { "lib/$_" })
foreach ($relativePath in $requiredPackagePaths) {
  $path = Join-Path $packageRoot ($relativePath -replace '/', '\')
  if (-not (Test-Path -LiteralPath $path)) {
    throw "missing packaged artifact: $relativePath"
  }
}

$repoHeaderCount = @(
  Get-ChildItem -LiteralPath (Join-Path $repoRoot "include\cgpui") `
    -Recurse -File
).Count
$packageHeaderCount = @(
  Get-ChildItem -LiteralPath (Join-Path $packageRoot "include\cgpui") `
    -Recurse -File
).Count
if ($repoHeaderCount -eq 0 -or $packageHeaderCount -ne $repoHeaderCount) {
  throw "Windows $Mode public header count mismatch"
}

Write-Output $packageRoot
