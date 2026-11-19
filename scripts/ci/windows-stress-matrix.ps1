param([string]$OutputDir = "artifacts/phase-j-stress", [int]$Scale = 1)
$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
$output = Join-Path $root "$OutputDir/windows.json"
New-Item -ItemType Directory -Force (Split-Path $output) | Out-Null
Push-Location $root
try {
  . (Join-Path $PSScriptRoot "windows-dependencies.ps1")
  $mcpp = Set-CgpuiWindowsDependencyEnvironment
  # The runner is the test program of tests/stress (arguments after --).
  & $mcpp test -p tests/stress -- --platform windows --output $output --scale $Scale
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  & python tools/gpui_parity/validate_stress_report.py `
      --profile docs/gpui-stress-profile.json --report $output
  exit $LASTEXITCODE
} finally { Pop-Location }
