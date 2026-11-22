[CmdletBinding()]
param(
  [Parameter(Mandatory = $true)]
  [ValidateSet("Debug", "Release")]
  [string]$Mode,
  [string]$OutputRoot = "",
  # MSVC runtime: MT (static CRT, the default, as dev builds) or MD (the DLL
  # CRT, for the SDK). mcpp has no debug CRT (MTd / MDd).
  [ValidateSet("", "MD", "MT")]
  [string]$Runtimes = ""
)

# Builds cgpui_app and hello_window with mcpp in a packaging workspace under
# the output root (sdk-package.py workspace: the repository's library and
# example members, with this package's CRT) and lays out the package.
$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

if ($env:OS -ne "Windows_NT") {
  throw "windows-package.ps1 requires Windows"
}

$repoRoot = [System.IO.Path]::GetFullPath(
  (Join-Path $PSScriptRoot "..\..")
)
$modeName = $Mode.ToLowerInvariant()
$profileName = if ($modeName -eq "debug") { "dev" } else { "release" }
$cxxRuntime = if ($Runtimes -eq "MD") { "toolchain-coupled" } else { "self-contained" }
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
$workspaceRoot = Join-Path $outputRoot "workspace"
$packageRoot = Join-Path $outputRoot "package"
$libRoot = Join-Path $packageRoot "lib"
$binRoot = Join-Path $packageRoot "bin"
$includeRoot = Join-Path $packageRoot "include"
New-Item -ItemType Directory -Force -Path @(
  $workspaceRoot, $packageRoot, $libRoot, $binRoot, $includeRoot
) | Out-Null

. (Join-Path $PSScriptRoot "windows-dependencies.ps1")
$mcpp = Set-CgpuiWindowsDependencyEnvironment

& python (Join-Path $PSScriptRoot "sdk-package.py") workspace `
  --repo-root $repoRoot --output $workspaceRoot --cxx-runtime $cxxRuntime
if ($LASTEXITCODE -ne 0) {
  throw "sdk-package.py workspace failed with exit code $LASTEXITCODE"
}
Push-Location $workspaceRoot
try {
  # Every member: selecting programs alone archives no library (mcpp links
  # a program from its dependencies' objects).
  & $mcpp build --profile $profileName
  if ($LASTEXITCODE -ne 0) {
    throw "mcpp build failed with exit code $LASTEXITCODE"
  }
} finally {
  Pop-Location
}

# The configuration directory mcpp built (target/<triple>/<fingerprint>).
$buildDir = @(
  Get-ChildItem -LiteralPath (Join-Path $workspaceRoot "target\x86_64-windows-msvc") `
    -Directory | Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName "build.ninja") }
)
if ($buildDir.Count -ne 1) {
  throw "expected one mcpp build directory under $workspaceRoot\target"
}
$binaryRoot = Join-Path $buildDir[0].FullName "bin"
Set-Content -LiteralPath (Join-Path $outputRoot "build-dir.txt") `
  -Value $buildDir[0].FullName -Encoding ascii

$libraries = [ordered]@{
  "cgpui_core.lib" = "core"
  "cgpui_platform.lib" = "platform"
  "cgpui_platform_win32.lib" = "platform-win32"
  "cgpui_renderer.lib" = "renderer"
  "cgpui_renderer_win32_composition.lib" = "renderer-win32-composition"
  "cgpui_renderer_vulkan.lib" = "renderer-vulkan"
  "cgpui_ui.lib" = "ui"
  "cgpui_app.lib" = "app"
}
foreach ($library in $libraries.Keys) {
  $source = Join-Path (Join-Path $binaryRoot $libraries[$library]) $library
  if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
    throw "missing Windows $Mode library: $source"
  }
  Copy-Item -LiteralPath $source -Destination $libRoot
}

$demo = Join-Path $binaryRoot "hello-window\hello_window.exe"
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
  libraries = @($libraries.Keys | ForEach-Object { "lib/$_" })
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
) + @($libraries.Keys | ForEach-Object { "lib/$_" })
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
