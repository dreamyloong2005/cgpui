[CmdletBinding()]
param(
  [string]$OutputRoot = "",
  [switch]$ReusePreparedRoot
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

if ($env:OS -ne "Windows_NT") {
  throw "windows-example-smoke.ps1 requires Windows"
}

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
  $OutputRoot = Join-Path $repoRoot "build\phase-g-ci\windows-examples-smoke"
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

if (-not $ReusePreparedRoot) {
  & (Join-Path $PSScriptRoot "windows-package.ps1") `
    -Mode Debug -OutputRoot $outputRoot
  if ($LASTEXITCODE -ne 0) {
    throw "Windows Debug package preparation failed with exit code $LASTEXITCODE"
  }
}

$manifestPath = Join-Path $outputRoot "package\manifest.json"
if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
  throw "prepared Windows package manifest is missing: $manifestPath"
}
$buildRoot = Join-Path $outputRoot "build-root"
$env:XMAKE_GLOBALDIR = Join-Path $outputRoot "global"
$env:XMAKE_CONFIGDIR = Join-Path $outputRoot "config"
$env:XMAKE_PKG_CACHEDIR = Join-Path $outputRoot "pkg-cache"
$env:XMAKE_PKG_INSTALLDIR = Join-Path $outputRoot "pkg-install"
if (-not (Test-Path -LiteralPath $buildRoot -PathType Container)) {
  throw "prepared Windows build root is missing: $buildRoot"
}

$targetsPath = Join-Path $PSScriptRoot "example-targets.txt"
$targets = @(Get-Content -LiteralPath $targetsPath | Where-Object {
  -not [string]::IsNullOrWhiteSpace($_) -and -not $_.StartsWith('#')
})
if ($targets.Count -eq 0) {
  throw "example target matrix is empty"
}

Push-Location $repoRoot
try {
  foreach ($target in $targets) {
    if ($target -notmatch '^api_parity_[a-z0-9_]+$') {
      throw "invalid example target: $target"
    }
    & xmake build -P $repoRoot -y -j 1 $target
    if ($LASTEXITCODE -ne 0) {
      throw "xmake build $target failed with exit code $LASTEXITCODE"
    }
    if ($target -eq "api_parity_hello_world") {
      $env:CGPUI_EXIT_AFTER_FIRST_FRAME = "1"
    }
    try {
      & xmake run -P $repoRoot $target
      if ($LASTEXITCODE -ne 0) {
        throw "xmake run $target failed with exit code $LASTEXITCODE"
      }
    } finally {
      Remove-Item Env:CGPUI_EXIT_AFTER_FIRST_FRAME -ErrorAction SilentlyContinue
    }
  }

  $registrationSmokes = @(
    "api_parity_public_animation_example",
    "api_parity_public_opacity_example",
    "api_parity_public_image_example",
    "api_parity_public_gif_viewer_example"
  )
  foreach ($target in $registrationSmokes) {
    & xmake test -P $repoRoot -j 1 -v "$target/*"
    if ($LASTEXITCODE -ne 0) {
      throw "xmake test $target failed with exit code $LASTEXITCODE"
    }
  }

  $demoSmokes = @(
    "windows_first_frame",
    "windows_resize_after_first_frame",
    "windows_close_after_first_frame",
    "windows_demo_smoke_flow"
  )
  foreach ($test in $demoSmokes) {
    & xmake test -P $repoRoot -j 1 -v "hello_window/$test"
    if ($LASTEXITCODE -ne 0) {
      throw "hello_window/$test failed with exit code $LASTEXITCODE"
    }
  }
} finally {
  Pop-Location
}

Write-Output "Windows example and smoke matrix passed"
