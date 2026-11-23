# Progress

## First tracer

Baseline c1b72784, existing public Renderer/RenderFrame pixel seam. Native
RemoveDevice is invoked on the actual adapter singleton; helper releases its
Device5/InfoQueue before calling the renderer. Both prior finished frames remain
alive, including an invalidated old image snapshot. The next frame must reuse
CPU image/text data without image reupload and match validated pre-loss pixels.

Executed under WARP + Debug:

```powershell
scripts/dev/test.ps1 -p tests/renderer-dx12 dx12_device_recovery
```

`recovery-red.log`: FAIL exit9,0.16s, "removed device did not complete recovery
before requesting retry". Current begin_frame returns a frame without observing
loss; no reconstruction exists. This is a feature tracer, not an arbitrary
driver-failure reproduction. Implementation began after this executed RED.

Frontend integration is in focused private frame/resize recovery leaves.
begin/present use typed retry only after full reconstruction. resize performs
the requested dimensions after recovery and never exposes a frame retry.

## Initial GREEN and reviews

`recovery-green.log`: original native tracer passes0.11s. `native-green.log`:
37/37 WARP+Debug programs pass, including five new recovery programs, prior
retirement fault injection and all existing drawing/capture tests.
`architecture-green.log`:202/202 including the new recovery owner/CPU-native
separation guards and CI matrix registration. `runtime-retry-green.log`: expanded
existing program passes12 root/child x begin/present x typed/fatal cases; no
factory recreation or after_frame callback on lost frames.

Independent Spec and native-lifecycle reviews report0 actionable findings.
Standards found one nonblocking duplicated test watchdog/debug-message filter;
the old removal test now reuses the new support without releasing its native
Device5/InfoQueue blockers early or broadening its three expected removal IDs.
Standards closure confirms0 outstanding findings. The final matrix rebuilds
and rechecks this test-only deduplication.
