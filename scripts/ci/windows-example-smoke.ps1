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
$buildRoot = Join-Path $outputRoot "workspace"
. (Join-Path $PSScriptRoot "windows-dependencies.ps1")
$mcpp = Set-CgpuiWindowsDependencyEnvironment
if (-not (Test-Path -LiteralPath $buildRoot -PathType Container)) {
  throw "prepared Windows packaging workspace is missing: $buildRoot"
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
    if ($target -eq "api_parity_hello_world") {
      $env:CGPUI_EXIT_AFTER_FIRST_FRAME = "1"
    }
    try {
      # mcpp run builds the program first.
      & $mcpp run -p examples/api_parity $target
      if ($LASTEXITCODE -ne 0) {
        throw "mcpp run $target failed with exit code $LASTEXITCODE"
      }
    } finally {
      Remove-Item Env:CGPUI_EXIT_AFTER_FIRST_FRAME -ErrorAction SilentlyContinue
    }
  }
  # The four examples that are also test programs (public_*_example/main).
  $registrationSmokes = @(
    "public_animation_example",
    "public_opacity_example",
    "public_image_example",
    "public_gif_viewer_example"
  )
  foreach ($test in $registrationSmokes) {
    & $mcpp test -p examples/api_parity $test
    if ($LASTEXITCODE -ne 0) {
      throw "mcpp test $test failed with exit code $LASTEXITCODE"
    }
  }
  $demoSmokes = @(
    "windows_first_frame",
    "windows_resize_after_first_frame",
    "windows_close_after_first_frame",
    "windows_demo_smoke_flow"
  )
  foreach ($test in $demoSmokes) {
    & $mcpp test -p examples/hello_window $test
    if ($LASTEXITCODE -ne 0) {
      throw "hello_window $test failed with exit code $LASTEXITCODE"
    }
  }
} finally {
  Pop-Location
}

Write-Output "Windows example and smoke matrix passed"
