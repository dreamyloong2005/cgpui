#Requires -Version 7.0
# Isolated runner regression fixture; no native renderer or GPU is used.
param(
    [Parameter(Mandatory)][string]$WorkspaceParent,
    [string]$Runner = (Join-Path $PSScriptRoot '../../../../scripts/dev/renderer-steady-state.ps1')
)
$ErrorActionPreference = 'Stop'
$parent = (Resolve-Path -LiteralPath $WorkspaceParent).Path
if (-not (Test-Path -LiteralPath $parent -PathType Container)) { throw 'WorkspaceParent must exist.' }
$workspace = Join-Path $parent ('runner-fixture-' + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $workspace | Out-Null
$runnerPath = (Resolve-Path -LiteralPath $Runner).Path
$helper = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot 'fake-child.ps1')).Path
$pwsh = (Get-Process -Id $PID).Path
$source = [IO.File]::ReadAllText($runnerPath)
$anchor = '$info.FileName = $exe'
if (($source.Split($anchor, [StringSplitOptions]::None)).Count -ne 2) {
    throw 'Expected exactly one executable assignment for fixture-only launcher adaptation.'
}
# The production runner deliberately has no test-fault option or launcher switch.
# Adapt only its external copy to launch a pure-PowerShell owned helper. The
# complete process/try/catch/finally/exit region stays byte-identical to the source.
$prefix = @('-NoLogo', '-NoProfile', '-NonInteractive', '-File', $helper)
$literalPrefix = ($prefix | ForEach-Object { "'" + $_.Replace("'", "''") + "'" }) -join ', '
$adapter = $source.Replace($anchor, $anchor + "`nforeach (`$prefixArgument in @($literalPrefix)) { `$info.ArgumentList.Add(`$prefixArgument) }")
$bodyAnchor = '$process = [Diagnostics.Process]::new()'
$bodyOffset = $source.IndexOf($bodyAnchor, [StringComparison]::Ordinal)
$adapterOffset = $adapter.IndexOf($bodyAnchor, [StringComparison]::Ordinal)
if ($bodyOffset -lt 0 -or $adapterOffset -lt 0 -or
    $source.Substring($bodyOffset) -cne $adapter.Substring($adapterOffset)) {
    throw 'Fixture must not alter the process or failure-handling code under test.'
}
$adaptedRunner = Join-Path $workspace 'runner-with-owned-helper.ps1'
[IO.File]::WriteAllText($adaptedRunner, $adapter)
$provenance = [ordered]@{
    fixture_only = $true; gpu_execution = $false; workspace = $workspace
    original_runner = $runnerPath; source_sha256 = (Get-FileHash -LiteralPath $runnerPath -Algorithm SHA256).Hash
    adapted_runner_sha256 = (Get-FileHash -LiteralPath $adaptedRunner -Algorithm SHA256).Hash
    helper_sha256 = (Get-FileHash -LiteralPath $helper -Algorithm SHA256).Hash
    unchanged_failure_handling_suffix = $true
    limitation = 'Launcher prefix is adapted in an external copy; no native binary/probe/report validation.'
}
$provenance | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $workspace 'provenance.json') -Encoding utf8
$cases = @()
$resultCode = 0
foreach ($case in @('success', 'evidence_io_failure', 'hard_timeout')) {
    $output = Join-Path $workspace $case
    $info = [Diagnostics.ProcessStartInfo]::new()
    $info.FileName = $pwsh
    $info.UseShellExecute = $false
    $info.CreateNoWindow = $true
    $info.RedirectStandardOutput = $true
    $info.RedirectStandardError = $true
    foreach ($argument in @('-NoLogo', '-NoProfile', '-NonInteractive', '-File', $adaptedRunner,
        '-Executable', $pwsh, '-Backend', 'dx12', '-AdapterKind', 'hardware', '-Profile', 'fixture-only',
        '-SourceLabel', 'fixture-only-no-gpu', '-Output', $output, '-RunDeadlineMs', '1',
        '-FrameDeadlineMs', '1', '-Warmup', '1', '-Samples', '1')) { $info.ArgumentList.Add($argument) }
    $info.Environment['CGPUI_RUNNER_FIXTURE_CASE'] = $case
    $owned = [Diagnostics.Process]::new()
    $owned.StartInfo = $info
    $started = $false
    $row = [ordered]@{ case = $case; passed = $false; runner_exit_code = $null; output = $output }
    try {
        $started = $owned.Start()
        if (-not $started) { throw 'Owned fixture runner failed to start.' }
        $stdout = $owned.StandardOutput.ReadToEndAsync()
        $stderr = $owned.StandardError.ReadToEndAsync()
        if (-not $owned.WaitForExit(45000)) { throw 'Owned fixture runner exceeded fixture deadline.' }
        $row.runner_exit_code = $owned.ExitCode
        [IO.File]::WriteAllText((Join-Path $workspace "$case.runner.stdout.log"), $stdout.GetAwaiter().GetResult())
        [IO.File]::WriteAllText((Join-Path $workspace "$case.runner.stderr.log"), $stderr.GetAwaiter().GetResult())
        $metadata = Get-Content -LiteralPath (Join-Path $output 'runner.json') -Raw | ConvertFrom-Json
        if ($metadata.exit_code -ne $row.runner_exit_code) { throw 'JSON/process exit-code mismatch.' }
        switch ($case) {
            'success' {
                if ($row.runner_exit_code -ne 0 -or $metadata.status -ne 'valid_baseline' -or
                    $metadata.child_exit_code -ne 0 -or $metadata.hard_timeout_occurred -or
                    -not (Test-Path -LiteralPath (Join-Path $output 'stdout.log') -PathType Leaf) -or
                    -not (Test-Path -LiteralPath (Join-Path $output 'stderr.log') -PathType Leaf)) {
                    throw 'Normal successful child/control did not remain successful.'
                }
            }
            'evidence_io_failure' {
                if ($row.runner_exit_code -eq 0 -or $row.runner_exit_code -eq 124 -or
                    $metadata.status -ne 'runner_failed' -or $metadata.child_exit_code -ne 0 -or
                    $metadata.hard_timeout_occurred -or -not $metadata.error -or
                    -not (Test-Path -LiteralPath (Join-Path $output 'stdout.log') -PathType Container)) {
                    throw 'Successful child followed by evidence IO failure was not truthfully rejected.'
                }
            }
            'hard_timeout' {
                if ($row.runner_exit_code -ne 124 -or $metadata.status -ne 'hard_timeout' -or
                    -not $metadata.hard_timeout_occurred -or $metadata.hard_timeout_ms -ne 15001) {
                    throw 'Known hard timeout lost explicit 124/status/timeout evidence.'
                }
            }
        }
        $row.passed = $true
    } catch {
        $resultCode = 1
        $row.error = $_.Exception.Message
    } finally {
        try {
            # This fixture's sole descendants are the explicitly-created runner
            # and its pure-PowerShell helper. Never enumerate unrelated processes.
            if ($started -and -not $owned.HasExited) {
                $owned.Kill($true)
                if (-not $owned.WaitForExit(5000)) { throw 'Owned fixture tree did not stop.' }
            }
        } catch { $resultCode = 1; $row.passed = $false; $row.cleanup_error = $_.Exception.Message }
        $owned.Dispose()
    }
    $cases += $row
}
[ordered]@{ fixture_only = $true; gpu_execution = $false; exit_code = $resultCode; cases = $cases } |
    ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $workspace 'fixture-results.json') -Encoding utf8
Write-Output "Fixture evidence retained at $workspace"
exit $resultCode
