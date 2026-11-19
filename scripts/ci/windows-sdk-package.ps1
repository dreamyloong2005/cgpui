[CmdletBinding()]
param(
  [Parameter(Mandatory = $true)]
  [ValidateSet("Debug", "Release")]
  [string]$Mode,
  [string]$OutputRoot = "build/release/ide-workbench",
  [switch]$KeepWorktree
)

# CGPUI SDK package (docs/sdk-packaging.md): builds windows-package.ps1
# with the /MD runtime in a temporary git worktree of HEAD, so only
# committed sources are packaged, then adds the SDK parts with
# sdk-package.py and archives with create-release-artifact.py.
$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

if ($env:OS -ne "Windows_NT") {
  throw "windows-sdk-package.ps1 requires Windows"
}

function Invoke-Checked {
  param([string]$What, [scriptblock]$Command)
  & $Command
  if ($LASTEXITCODE -ne 0) {
    throw "$What failed with exit code $LASTEXITCODE"
  }
}

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$modeName = $Mode.ToLowerInvariant()
$runtime = if ($modeName -eq "debug") { "MDd" } else { "MD" }
if (-not [System.IO.Path]::IsPathRooted($OutputRoot)) {
  $OutputRoot = Join-Path $repoRoot $OutputRoot
}
$outputRoot = [System.IO.Path]::GetFullPath($OutputRoot)
$buildPrefix = (Join-Path $repoRoot "build").TrimEnd('\') + '\'
if (-not $outputRoot.StartsWith(
      $buildPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
  throw "output root must stay inside the ignored build directory: $outputRoot"
}
$packer = Join-Path $PSScriptRoot "sdk-package.py"
$archiver = Join-Path $PSScriptRoot "create-release-artifact.py"
$packageName = "cgpui-sdk-windows-x64-$modeName"
$worktree = Join-Path $outputRoot "worktree-$modeName"
$finalPackage = Join-Path $outputRoot "windows-$modeName"
$archive = Join-Path $outputRoot "$packageName.zip"

# Tracked changes would not reach the worktree: refuse them. Untracked
# files stay in this checkout and are not packaged.
Invoke-Checked "clean-tree check" {
  & python $packer check-clean --repo-root $repoRoot --allow-untracked
}
$commit = (& git -C $repoRoot rev-parse HEAD).Trim()
New-Item -ItemType Directory -Force -Path $outputRoot | Out-Null
if (Test-Path -LiteralPath $worktree) {
  Invoke-Checked "git worktree remove" {
    & git -C $repoRoot worktree remove --force $worktree
  }
}
Invoke-Checked "git worktree add" {
  & git -C $repoRoot worktree add --detach $worktree $commit
}
try {
  Invoke-Checked "worktree clean-tree check" {
    & python $packer check-clean --repo-root $worktree
  }
  $packageScript = Join-Path $worktree "scripts\ci\windows-package.ps1"
  & $packageScript -Mode $Mode -OutputRoot "build\sdk-package" `
    -Runtimes $runtime | Out-Host
  $buildOutput = Join-Path $worktree "build\sdk-package"
  $binaryRoot = Join-Path $buildOutput "build-root\windows\x64\$modeName"
  $linkLog = Join-Path $buildOutput "example_workbench-link.log"
  # The verbose example_workbench link line gives the consumer link order.
  # Run inside the worktree: the configured build directory is relative.
  Push-Location $worktree
  try {
    & xmake build -P $worktree -v example_workbench 2>&1 |
      Out-File -LiteralPath $linkLog -Encoding utf8
  } finally {
    Pop-Location
  }
  if ($LASTEXITCODE -ne 0) {
    Get-Content -LiteralPath $linkLog -Tail 20 | Out-Host
    throw "xmake build example_workbench failed with exit code $LASTEXITCODE"
  }
  $workPackage = Join-Path $buildOutput "package"
  Invoke-Checked "sdk-package.py finalize" {
    & python $packer finalize --platform windows --mode $modeName `
      --runtime $runtime --repo-root $worktree --binary-root $binaryRoot `
      --link-log $linkLog --package-root $workPackage `
      --executable hello_window --executable example_workbench
  }
  if (Test-Path -LiteralPath $finalPackage) {
    Remove-Item -LiteralPath $finalPackage -Recurse -Force
  }
  Copy-Item -LiteralPath $workPackage -Destination $finalPackage -Recurse
} finally {
  if (-not $KeepWorktree) {
    & git -C $repoRoot worktree remove --force $worktree
    & git -C $repoRoot worktree prune
  }
}

$env:SOURCE_DATE_EPOCH = "1704067200"
Invoke-Checked "create-release-artifact.py" {
  & python $archiver --platform windows --mode $modeName `
    --package-root $finalPackage --package-name $packageName `
    --archive-name "$packageName.zip" --output $archive
}
# .NET directly: Get-FileHash is missing when PSModulePath points at
# another PowerShell edition.
$stream = [System.IO.File]::OpenRead($archive)
try {
  $digest = [System.Security.Cryptography.SHA256]::Create().ComputeHash($stream)
} finally {
  $stream.Dispose()
}
$hash = -join ($digest | ForEach-Object { $_.ToString("x2") })
Set-Content -LiteralPath "$archive.sha256" -Encoding ascii `
  -Value "$hash  $packageName.zip"
Write-Output "$archive $hash"
