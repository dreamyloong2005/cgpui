param([string]$OutputDir = "artifacts/phase-j-stress", [int]$Scale = 1)
$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
$output = Join-Path $root "$OutputDir/windows.json"
New-Item -ItemType Directory -Force (Split-Path $output) | Out-Null
Push-Location $root
try {
  & xmake build -j 1 phase_j_stress_runner
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  & xmake run phase_j_stress_runner --platform windows --output $output --scale $Scale
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  & python tools/gpui_parity/validate_stress_report.py `
      --profile docs/gpui-stress-profile.json --report $output
  exit $LASTEXITCODE
} finally { Pop-Location }
