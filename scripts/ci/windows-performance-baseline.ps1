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
  & xmake build -j 1 phase_j_performance_runner
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  & xmake run phase_j_performance_runner --platform windows --output $output --iterations $Iterations
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  & python tools/gpui_parity/validate_performance_baseline.py `
      --policy docs/gpui-performance-baseline-policy.json --report $output
  exit $LASTEXITCODE
} finally {
  Pop-Location
}
