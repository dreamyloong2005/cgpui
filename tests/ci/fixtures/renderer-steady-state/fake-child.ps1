#Requires -Version 7.0
# Fixture-owned helper only. No GPU/native renderer/compiler; no child processes.
# Intentionally no param block: consume the runner's ordinary --key/value args.
$ErrorActionPreference = 'Stop'
$probeOutput = $null
for ($index = 0; $index -lt $args.Count - 1; ++$index) {
    if ($args[$index] -eq '--output') { $probeOutput = $args[$index + 1]; break }
}
if (-not $probeOutput) { throw 'Fixture helper did not receive the runner output argument.' }
$case = [Environment]::GetEnvironmentVariable('CGPUI_RUNNER_FIXTURE_CASE')
if ($case -notin @('success', 'evidence_io_failure', 'hard_timeout')) { throw 'Unknown fixture case.' }
[Console]::Out.WriteLine("FIXTURE_CHILD case=$case")
[Console]::Out.Flush()
if ($case -eq 'evidence_io_failure') {
    # Successful child leaves an intentional directory where the runner will
    # write its stdout file. runner.json remains writable in the owned directory.
    $runRoot = [IO.Path]::GetDirectoryName([IO.Path]::GetFullPath($probeOutput))
    New-Item -ItemType Directory -Path (Join-Path $runRoot 'stdout.log') | Out-Null
}
if ($case -eq 'hard_timeout') { Start-Sleep -Seconds 60 }
exit 0
