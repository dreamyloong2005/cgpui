[CmdletBinding()]
param(
  [string]$OutputRoot = "build/phase-j-release/windows",
  [switch]$SkipPackageBuild
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
if (-not [System.IO.Path]::IsPathRooted($OutputRoot)) {
  $OutputRoot = Join-Path $repoRoot $OutputRoot
}
$outputRoot = [System.IO.Path]::GetFullPath($OutputRoot)
$repoPrefix = $repoRoot.TrimEnd('\', '/') + [IO.Path]::DirectorySeparatorChar
if ($outputRoot -eq $repoRoot -or
    -not $outputRoot.StartsWith($repoPrefix, [StringComparison]::OrdinalIgnoreCase)) {
  throw "output root must stay inside the repository: $outputRoot"
}

if (-not $SkipPackageBuild) {
  & (Join-Path $PSScriptRoot "windows-package.ps1") `
    -Mode Release -OutputRoot $outputRoot
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$packageRoot = Join-Path $outputRoot "package"
if (-not (Test-Path -LiteralPath (Join-Path $packageRoot "manifest.json") -PathType Leaf)) {
  throw "missing release package manifest: $packageRoot"
}
$artifactRoot = Join-Path $outputRoot "artifacts"
$archive = Join-Path $artifactRoot "cgpui-windows-release.zip"
$verificationArchive = Join-Path $artifactRoot "cgpui-windows-release.verify.zip"
New-Item -ItemType Directory -Force $artifactRoot | Out-Null
$env:SOURCE_DATE_EPOCH = "1704067200"

& python (Join-Path $PSScriptRoot "create-release-artifact.py") `
  --platform windows --package-root $packageRoot `
  --package-name cgpui-windows-release `
  --archive-name cgpui-windows-release.zip --output $archive
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& python (Join-Path $PSScriptRoot "create-release-artifact.py") `
  --platform windows --package-root $packageRoot `
  --package-name cgpui-windows-release `
  --archive-name cgpui-windows-release.zip --output $verificationArchive
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$first = (Get-FileHash -Algorithm SHA256 -LiteralPath $archive).Hash
$second = (Get-FileHash -Algorithm SHA256 -LiteralPath $verificationArchive).Hash
$difference = Compare-Object -ReferenceObject @($first) -DifferenceObject @($second)
if ($difference) { throw "Windows release archive is not reproducible" }
Remove-Item -LiteralPath $verificationArchive
Copy-Item -LiteralPath (Join-Path $packageRoot "release-manifest.json") `
  -Destination $artifactRoot
Write-Output $archive
