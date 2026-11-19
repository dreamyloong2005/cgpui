[CmdletBinding()]
param(
  [Parameter(Mandatory = $true)]
  [ValidateSet("Debug", "Release")]
  [string]$Mode,
  [string]$OutputRoot = "build/release/ide-workbench",
  [switch]$KeepWorktree
)

# CGPUI SDK package (docs/sdk-packaging.md): builds windows-package.ps1
# (mcpp) with the /MD runtime in a temporary git worktree of HEAD, so only
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
# mcpp has no debug CRT: both packages use the release DLL CRT.
$runtime = "MD"
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
  if ($LASTEXITCODE -ne 0) {
    throw "windows-package.ps1 failed with exit code $LASTEXITCODE"
  }
  $buildOutput = Join-Path $worktree "build\sdk-package"
  $buildDir = (Get-Content -LiteralPath (Join-Path $buildOutput "build-dir.txt")).Trim()
  $profileName = if ($modeName -eq "debug") { "dev" } else { "release" }
  . (Join-Path $PSScriptRoot "windows-dependencies.ps1")
  $mcpp = Set-CgpuiWindowsDependencyEnvironment
  $workPackage = Join-Path $buildOutput "package"
  Invoke-Checked "sdk-package.py finalize" {
    & python $packer finalize --platform windows --mode $modeName `
      --profile $profileName --runtime $runtime --repo-root $worktree `
      --workspace (Join-Path $buildOutput "workspace") --build-dir $buildDir `
      --mcpp $mcpp --package-root $workPackage `
      --executable hello-window/hello_window `
      --executable example-workbench/example_workbench
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
