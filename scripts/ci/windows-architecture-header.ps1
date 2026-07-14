[CmdletBinding()]
param(
  [string]$OutputRoot = "",
  [switch]$ReusePreparedRoot
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

if ($env:OS -ne "Windows_NT") {
  throw "windows-architecture-header.ps1 requires Windows"
}

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
  $OutputRoot = Join-Path $repoRoot "build\phase-g-ci\windows-architecture-header"
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

$manifest = Join-Path $outputRoot "package\manifest.json"
if (-not (Test-Path -LiteralPath $manifest -PathType Leaf)) {
  throw "prepared Windows package manifest is missing: $manifest"
}
. (Join-Path $PSScriptRoot "windows-dependencies.ps1")
Set-CgpuiWindowsDependencyEnvironment
$env:XMAKE_CONFIGDIR = Join-Path $outputRoot "config"

$matrix = Join-Path $PSScriptRoot "architecture-header-targets.txt"
$targets = @()
foreach ($line in Get-Content -LiteralPath $matrix) {
  if ([string]::IsNullOrWhiteSpace($line) -or $line.StartsWith('#')) { continue }
  $parts = $line.Split('|')
  if ($parts.Count -ne 2 -or $parts[0] -notmatch '^[a-z0-9_]+$') {
    throw "invalid architecture/header matrix entry: $line"
  }
  $targets += $parts[0]
}
if ($targets.Count -eq 0) { throw "architecture/header target matrix is empty" }

Push-Location $repoRoot
try {
  foreach ($target in $targets) {
    & xmake build -P $repoRoot -y -j 1 $target
    if ($LASTEXITCODE -ne 0) {
      throw "xmake build $target failed with exit code $LASTEXITCODE"
    }
  }
  $filters = @($targets | ForEach-Object { "$_/*" })
  & xmake test -P $repoRoot -j 1 -v @filters
  if ($LASTEXITCODE -ne 0) {
    throw "Windows architecture/header tests failed with exit code $LASTEXITCODE"
  }
} finally {
  Pop-Location
}

Write-Output "Windows architecture/header matrix passed ($($targets.Count) targets)"
