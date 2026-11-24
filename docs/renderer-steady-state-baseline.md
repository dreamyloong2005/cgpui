# Renderer steady-state observations (Windows)

This opt-in native probe measures explicit DX12 or Vulkan API wall-clock envelopes.
It is not an FPS, GPU-time, scanout, input-latency or performance-pass test. It does
not change production pacing or renderer policy. Default/auto selection is not used.

## Build and run

Use PowerShell 7 and the normal mcpp drivers from the repository root:

```powershell
$env:MCPP_JOBS='4'
pwsh -NoProfile -File scripts/dev/build.ps1 -p tests/renderer-steady-state
pwsh -NoProfile -File scripts/dev/test.ps1 -p tests/renderer-steady-state
```

The native `renderer_steady_state_probe` executable is a manual bin, deliberately
absent from test discovery. The four discovered tests cover only pure statistics,
creation/schema outcomes and display-facts equality. Use the executable from this
build's target/profile directory; do not take the first stale binary from a search.
Record the actual compiler flags, source commit/diff, driver/OS versions and power
conditions alongside results. The runner hashes the executable; profile/source
labels supplied by the caller remain explicitly unverified metadata.

Create an output parent, set `$exe` to that verified executable and run serially:

```powershell
pwsh -NoProfile -File scripts/dev/renderer-steady-state.ps1 `
  -Executable $exe -Backend dx12 -AdapterKind hardware -Dx12Adapter 0 `
  -Profile dev -SourceLabel $verifiedSource -Output "$evidence/dx12-mixed-01" `
  -Scene mixed -Warmup 120 -Samples 600
```

Use a fresh output directory for every attempt. `-Backend vulkan` must omit
`-Dx12Adapter`; its actual adapter must be observed, not assumed equal to DX12.
Pin subsequent comparable runs with `-ExpectedLuid <decimal>`. Use `-Dx12Adapter
warp -AdapterKind software` for WARP, and keep it separate from hardware results.
Debug-layer/GPU-validation runs and release/dev profiles are distinct cohorts.
The three scenes are `rect-grid` (128 rectangles), `cached-images` (32 images) and
`mixed` (both). Record at least three complete runs under unchanged premises
before describing repeatability; preserve failed/partial runs too.

The probe intentionally displays one native 800x600 window without taking focus
or making it topmost. Do not cover, move, minimize or close it during a valid run.
Display/window changes invalidate the whole series, including a notified change
and return. Five-point occlusion checks and sampled facts are bounded evidence,
not proof of every desktop pixel or unnotified between-sample driver changes.

For formal comparisons, build and verify a separate release executable:

```powershell
pwsh -NoProfile -File scripts/dev/build.ps1 --release -p tests/renderer-steady-state
# Set $releaseExe to this build's verified release output, not the dev binary.
pwsh -NoProfile -File scripts/dev/renderer-steady-state.ps1 `
  -Executable $releaseExe -Backend dx12 -AdapterKind hardware -Dx12Adapter 0 `
  -Profile release -SourceLabel $verifiedSource -Output "$evidence/dx12-release-mixed-01" `
  -Scene mixed -Warmup 120 -Samples 600
```

Verify actual optimization flags and binary provenance; `-Profile release` does
not build or select a release binary. Dev observations remain a separate cohort.

## Outputs and interpretation

`runner.json` records process exit, actual executable hash, timeout and evidence-IO
failure. Native `probe/report.json` schema3 and `probe/samples.csv` record:

- Separate preflight capture and fresh measured-renderer creation wall time.
  `created` only means factory success, not whole-run validity; preflight may have
  warmed process/driver state. No returned duration exists after a killed factory.
- One renderer-cold uncaptured row; fixed warm-up; fixed steady uncaptured rows;
  a fresh final capture outside measurement. Both pixel checks must pass.
- Actual backend/selection/adapter LUID/software/generation, display facts and
  notification epochs; zero steady image/glyph uploads and exact expected work.
- All steady rows and nearest-rank p50/p95/min/max/mean, with no discarded outliers.

DX12 currently completes submitted work synchronously; uncaptured Vulkan may
retire prior work on the next frame. Stage labels therefore are not equivalent
GPU work. Diagnostic batch counts also differ: DX12 counts visible draw groups;
Vulkan's current legacy batch planner excludes image commands. An image-only
Vulkan scene therefore reports zero batches despite submitting all image draws.
Use the separate exact draw counts and pixel checks; do not compare these batch
counts as native submission counts or optimization gains.
API envelopes include waits, but exclude reporting, diagnostic copying,
frame destruction and full display checks outside the timing boundary. Never
convert allocation counts or these wall-clock numbers into an FPS/speedup claim.

Per-frame/cooperative run deadlines are backed by an owned-process hard deadline
(run deadline plus 15 seconds). Timeout exits124; no raw Present retry, automatic
backend fallback, sample skipping or successful status after evidence-IO failure
is allowed. A valid baseline is a measurement, not an invented performance target.

## Runner regression and negative controls

The non-GPU fixture preserves evidence in a new GUID directory below an existing
parent and tests successful child, child-success followed by evidence-IO failure,
and a real owned-helper timeout:

```powershell
pwsh -NoProfile -File tests/ci/fixtures/renderer-steady-state/check-runner.ps1 `
  -WorkspaceParent $evidence
```

It adapts only an external runner copy's launcher to invoke PowerShell; the entire
process/failure-handling suffix stays identical and hashes are recorded. Its
synthetic `valid_baseline` cannot certify native report compatibility or timing.
Root/native acceptance separately needs wrong-adapter identity and actual window
visibility/change controls. Retain their nonzero exits and partial reports; never
weaken the observation guards to obtain a baseline. Windows measurements do not
certify native macOS, all drivers, release packaging or overall DX12 completion.
