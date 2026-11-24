#Requires -Version 7.0
# Explicit native measurement: verify the actual mcpp binary/profile before running.
param(
    [Parameter(Mandatory)][string]$Executable,
    [Parameter(Mandatory)][ValidateSet('dx12','vulkan')][string]$Backend,
    [Parameter(Mandatory)][ValidateSet('hardware','software')][string]$AdapterKind,
    [Parameter(Mandatory)][string]$Profile,
    [Parameter(Mandatory)][string]$Output,
    [string]$Dx12Adapter = '',
    [string]$ExpectedLuid = '',
    [string]$SourceLabel = 'unverified',
    [ValidateSet('mixed','rect-grid','cached-images')][string]$Scene = 'mixed',
    [ValidateRange(1,100000)][int]$Warmup = 120,
    [ValidateRange(1,100000)][int]$Samples = 600,
    [ValidateRange(1,60000)][int]$FrameDeadlineMs = 10000,
    [ValidateRange(1,3600000)][int]$RunDeadlineMs = 180000,
    [switch]$DebugLayer,
    [switch]$GpuValidation
)
$ErrorActionPreference = 'Stop'
$exe = (Resolve-Path -LiteralPath $Executable).Path
$root = [IO.Path]::GetFullPath($Output)
if (Test-Path -LiteralPath $root) { throw 'Output must be a new run directory; no overwrite.' }
if (-not (Test-Path -LiteralPath ([IO.Path]::GetDirectoryName($root)) -PathType Container)) {
    throw 'The output parent must already exist.'
}
if ($Backend -ne 'dx12' -and $Dx12Adapter) { throw 'Dx12Adapter cannot select a Vulkan adapter.' }
New-Item -ItemType Directory -Path $root | Out-Null
$meta = [ordered]@{
    schema_version = 1; status = 'starting'; executable = $exe
    executable_sha256 = (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash
    profile_label_unverified = $Profile; source_label_unverified = $SourceLabel
    os_version = [Environment]::OSVersion.VersionString
    start_utc = [DateTime]::UtcNow.ToString('o'); exit_code = $null; child_exit_code = $null
    hard_timeout_ms = $RunDeadlineMs + 15000; hard_timeout_occurred = $false; pid = $null
    no_speedup_claim = $true
}
$info = [Diagnostics.ProcessStartInfo]::new()
$info.FileName = $exe
$info.UseShellExecute = $false
$info.CreateNoWindow = $true # No helper console; the intentional native test HWND is visible.
$info.RedirectStandardOutput = $true
$info.RedirectStandardError = $true
foreach ($argument in @('--backend', $Backend, '--adapter-kind', $AdapterKind,
    '--profile', $Profile, '--source-label', $SourceLabel,
    '--output', (Join-Path $root 'probe'), '--scene', $Scene,
    '--warmup', "$Warmup", '--samples', "$Samples",
    '--frame-deadline-ms', "$FrameDeadlineMs", '--run-deadline-ms', "$RunDeadlineMs")) {
    $info.ArgumentList.Add($argument)
}
if ($ExpectedLuid) { $info.ArgumentList.Add('--expected-luid'); $info.ArgumentList.Add($ExpectedLuid) }
$info.Environment['CGPUI_RENDERER'] = $Backend
if ($Dx12Adapter) { $info.Environment['CGPUI_DX12_ADAPTER'] = $Dx12Adapter }
$info.Environment['CGPUI_DX12_DEBUG'] = [string][int]($DebugLayer -or $GpuValidation)
$info.Environment['CGPUI_DX12_GPU_VALIDATION'] = [string][int]$GpuValidation.IsPresent
$process = [Diagnostics.Process]::new()
$process.StartInfo = $info
$exitCode = 1
try {
    if (-not $process.Start()) { throw 'Probe process failed to start.' }
    $meta.pid = $process.Id
    $stdout = $process.StandardOutput.ReadToEndAsync()
    $stderr = $process.StandardError.ReadToEndAsync()
    if (-not $process.WaitForExit($meta.hard_timeout_ms)) {
        # Record timeout before cleanup so a later failure cannot erase exit 124.
        $meta.hard_timeout_occurred = $true
        $meta.status = 'hard_timeout'
        $exitCode = 124
        # Kill only the process object created here. Never enumerate/kill unrelated tests.
        $process.Kill()
        if (-not $process.WaitForExit(5000)) { throw 'Owned probe did not terminate after timeout.' }
    } else {
        $exitCode = $process.ExitCode
        $meta.status = if ($exitCode -eq 0) { 'valid_baseline' } else { 'probe_failed' }
    }
    $meta.child_exit_code = $process.ExitCode
    [IO.File]::WriteAllText((Join-Path $root 'stdout.log'), $stdout.GetAwaiter().GetResult())
    [IO.File]::WriteAllText((Join-Path $root 'stderr.log'), $stderr.GetAwaiter().GetResult())
} catch {
    # Child success does not mean runner success when required evidence IO fails.
    $exitCode = if ($meta.hard_timeout_occurred) { 124 } else { 1 }
    $meta.status = 'runner_failed'
    $meta.error = $_.Exception.Message
    try {
        if ($meta.pid -and -not $process.HasExited) {
            $process.Kill()
            if (-not $process.WaitForExit(5000)) { throw 'Owned probe cleanup did not finish.' }
        }
    } catch {
        $exitCode = if ($meta.hard_timeout_occurred) { 124 } else { 1 }
        $meta.cleanup_error = $_.Exception.Message
    }
} finally {
    try { $process.Dispose() } catch {
        $exitCode = if ($meta.hard_timeout_occurred) { 124 } else { 1 }
        $meta.status = 'runner_failed'
        $meta.dispose_error = $_.Exception.Message
    }
    $meta.exit_code = $exitCode
    $meta.end_utc = [DateTime]::UtcNow.ToString('o')
    try {
        $meta | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $root 'runner.json') -Encoding utf8
    } catch {
        $exitCode = if ($meta.hard_timeout_occurred) { 124 } else { 1 }
        $meta.status = 'runner_failed'
        $meta.exit_code = $exitCode
        $meta.metadata_error = $_.Exception.Message
        # A blocked JSON target cannot preserve itself. Report on stderr and fail,
        # never claim that runner.json was saved or silently return child success.
        [Console]::Error.WriteLine(($meta | ConvertTo-Json -Depth 8 -Compress))
    }
}
exit $exitCode
