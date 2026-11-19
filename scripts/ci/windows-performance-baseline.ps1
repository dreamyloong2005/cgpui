param(
  [string]$OutputDir = "artifacts/phase-j-performance",
  [int]$Iterations = 64
)

$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
$output = Join-Path $root "$OutputDir/windows.json"
New-Item -ItemType Directory -Force (Split-Path $output) | Out-Null
Push-Location $root
try {
  . (Join-Path $PSScriptRoot "windows-dependencies.ps1")
  $mcpp = Set-CgpuiWindowsDependencyEnvironment
  # The runner is the test program of tests/performance; mcpp test passes
  # the arguments after -- to it.
  & $mcpp test -p tests/performance -- --platform windows --output $output --iterations $Iterations
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  & python tools/gpui_parity/validate_performance_baseline.py `
      --policy docs/gpui-performance-baseline-policy.json --report $output
  exit $LASTEXITCODE
} finally {
  Pop-Location
}
