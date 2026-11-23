# Next slice: native WindowRuntime device-recovery acceptance

Date: 2026-11-24. Status: read-only design; **not implemented or run**.
This note is excluded from the current frozen-source matrix. It does not change
that milestone or claim full DX12 recovery completion.

## What is already implemented

The actual runtime consumes `ErrorCode::frame_retry_required` and queues another
render on the affected window instead of treating it as a successful frame:

- `src/ui/runtime_frame_retry.cpp:8-12` calls `Window(runtime, id).request_render()`.
- `src/ui/runtime_rendering.cpp:12-17` routes root-window errors through it.
- `src/ui/runtime_window_rendering.cpp:6-13` does the same for child windows.
- `src/ui/runtime_renderer_frame_results.cpp` and
  `src/ui/runtime_window_frame_results.cpp` publish after-frame callbacks and
  successful-frame diagnostics only after `render_view` succeeds.
- `tests/ui/window_runtime_frame_retry_test.cpp` already tests root/child,
  begin/present and typed-retry/fatal outcomes, but its renderer/platform are
  synthetic.

Existing native DX12 recovery tests independently verify real idle-device
`RemoveDevice`, reconstruction, retained images/glyphs and pixels through direct
Renderer calls. The missing acceptance seam is the complete **native Win32 event
loop + actual DX12 + normal runtime retry scheduling**. Do not add a second retry
mechanism, return false successful presents, or rewrite existing runtime logic
unless a real failing test demonstrates a defect.

## Why public TestApp is not the fixture

`src/ui/test_app.cpp:12-22` fixes TestApp to TestPlatformApplication/TestRenderer;
`src/ui/test_app_rendering.cpp:26-31` obtains its render snapshot by dynamic_cast
only to TestRenderer. Its public constructor has no native renderer/platform
injection. Its counters therefore cannot be presented as native DX12 recovery.
Do not expand the production TestApp API merely to make this test convenient.

Instead use:

1. `create_platform_application()` for the real Win32 implementation.
2. `run_app(application, root_view, AppRendererFactory, options)` for actual
   runtime-owned native windows and renderer lifetime.
3. `options.setup_context` to install after-frame/error callbacks and create a
   pending ordinary child with `runtime.try_open_window(options, owned_view)`.
   Existing app-runner tests cover pre-run pending children; `runtime.run()`
   activates them after root native creation.
4. Normal `Window(runtime, target_id).request_render()` for every requested
   redraw. Never manually call runtime `try_draw_frame`, emit synthetic
   WindowRedrawRequested, or loop on Renderer::begin_frame to obtain recovery.

`WindowRuntimeContext` publicly supplies Renderer&, PlatformWindow&,
window_runtime_id, scale and application. Its `renderer.last_frame_pixels()`,
`last_frame_diagnostic_snapshot()` and `backend_info()` are the observation seam.
Its platform window exposes native_surface() for HWND/thread checks. No private
runtime record or DX12 state mutation is necessary.

## Minimal capture observer, not a fake backend

The existing runtime does not request pixel capture on its RenderFrame. Use a
**test-only Renderer observer** that owns a renderer returned by the real
`create_dx12_renderer(descriptor)` factory:

- `begin_frame()` increments an external attempt counter, calls the actual
  renderer, and records an actual returned `frame_retry_required` separately
  from other errors. Return the original error unchanged.
- On a real successful frame, call that frame's public
  `request_pixel_capture()`; propagate capture errors. Return that same frame
  object to runtime. No RenderFrame wrapper is required.
- Forward resize, backend_info, last pixels, last diagnostics, composition,
  blur support, popup support and transparent intent to the underlying renderer.
- Never manufacture a backend identity, generation, successful present, pixels,
  diagnostics or retry result. The native backend is the only source of those.
- Count factory calls outside the observer. External counters/phase state
  outlive `run_app`; avoid retaining observer pointers beyond runtime teardown.
- Do not obtain validation pixels by drawing an extra frame from after-frame:
  that would test a different frame and could hide the runtime's actual failure.

The observer itself neither acquires nor retains native COM device references.
Use `dx12_remove_native_device(info)` from the existing recovery test helper;
its temporary native/InfoQueue references must die before the recovery call.
Reuse the existing bounded standalone-test process watchdog, not a production
resource-release timeout.

## Scene and once-uploaded ownership

Each View owns a stable ImageAsset member: a 2x2 RGBA image with four different
opaque colors, distinct asset IDs per window. PaintList::upload_image retains a
pointer until render_view submits commands **after paint() returns**, so a local
ImageAsset temporary in paint() would be a dangling test fixture.

On the View's first actual paint, enqueue `upload_image(member)` and increment a
submission-intent counter; never enqueue it again. An unsuccessful begin does
not call View::paint, so it must not consume this first-upload opportunity.
Every paint draws:

- An opaque background rectangle to make captures independent of runtime clear.
- The same four-color image with nearest sampling, well inside the viewport.
- A stable string such as `M` through PaintList::fill_text.
- A separate epoch-color rectangle, changed before device removal, so old pixels
  cannot satisfy post-recovery assertions.

Use context.scale to translate sample positions into device pixels. Do not
assume physical monitor DPI is 1. Verify the viewport is large enough for the
scene. Match image quadrant centers, epoch marker and glyph coverage (or a
saved same-scale glyph ROI) without claiming full framebuffer comparison.

## Two independent native programs

Suggested entries:

- `dx12_runtime_root_recovery_test.cpp`: request the root first after removal.
- `dx12_runtime_child_recovery_test.cpp`: request the ordinary child first.

Both programs use two native ordinary HWNDs and two real DX12 renderers on the
same UI thread. Assert both windows' GetWindowThreadProcessId equals the current
thread, both reports identify DX12 and the same adapter LUID, and factory count
is exactly two. The other ordinary renderer is a real same-thread cohort peer,
not an external manually pinned native device.

A shared helper can parameterize which window initiates the requested redraw;
keep capture observation, scene and event-loop phase machine in focused test
leaves rather than one large header. Tests remain Windows-only. Keeping them
in tests/renderer-dx12 requires adding `cgpui.ui` to that suite's Windows-only
dependencies and registering both discovery entries; production renderer
packages do not gain a UI dependency. This keeps the existing per-adapter native
matrix selector covering the new programs.

## Phase machine and pass/fail oracles

1. **Warm both windows.** Let the real message loop paint them. If a child's first
   paint is not naturally queued, request_render once through its Window handle.
   Record actual pixels, native diagnostics, generation, LUID, per-window success
   counts and the cold glyph upload byte count. Require each View upload counter
   equals one. Both actual renderers must already exist before removal.
2. **Remove and request.** Once both warm frames pass, copy the baseline reports,
   advance both epoch colors, call the real RemoveDevice helper, and normally
   request the chosen root/child target's render. Do not hold any old native COM
   reference. Do not destroy the peer to make reconstruction easier.
3. **Observe runtime retry.** The observer must see an actual typed begin retry,
   not infer it from an error string. Runtime should return to its native event
   loop and schedule the next paint. No test-side begin/present retry loop.
4. **Observe fresh target frame.** Its after-frame callback must show generation
   baseline+1, same LUID, new epoch marker, original image colors and glyphs.
   Require actual image reupload bytes=16 and glyph reupload bytes equal the
   saved cold glyph payload for the same cached scene. Factory count remains 2;
   View upload count remains 1. Request the peer through its own Window handle.
5. **Observe peer recovery.** Require the same native results on the peer, also
   without factory recreation or a second application upload. This exercises
   lazy same-thread cohort reconstruction rather than merely single-window loss.
6. **Warm-cache control.** Request another ordinary render of each window and
   require unchanged new-generation pixels with zero image/glyph upload bytes.
   Quit normally only after all required flags are satisfied.
7. **After run.** Require run result 0, no runtime error callback, both recovered
   and warm-control flags, factory count exactly 2 and once-upload counters 1.
   Check the recovered device debug queue with the existing helper while the
   actual renderer is still alive, not through a dangling observer after run.

Real Windows may deliver additional WM_PAINT events because of exposure/focus.
Do **not** require a global exact count of all frames or a single fixed root/child
callback ordering. Track generation transitions and the first successful frame
per required phase. Consume the first recovered-frame upload oracle before later
cache hits overwrite the latest diagnostic snapshot. The first peer operation
may occur due to an OS repaint; accept correct real cohort recovery rather than
mistaking it for a test failure. Both windows still must observe the typed retry
and independently satisfy all fresh-pixel/once-upload/factory invariants.

For the planned begin-loss variants, assert exactly one actual typed begin retry
per window for this single removed generation, while allowing additional normal
successful paints. If an unexpected resize recovery occurs before begin and
therefore consumes loss instead, diagnose that event sequencing explicitly;
do not silently relax the typed-retry oracle or claim this seam was exercised.

An after-frame callback on a rejected frame is a failure: captures are cleared
by recovery and the old successful diagnostic must not be republished as a new
successful frame. The first callback after removal must therefore pass the
fresh-generation/new-marker oracle. A missing child wakeup, fatal retry handling,
failed cohort retirement or lost CPU cache must fail via an explicit assertion,
nonzero run result, or the standalone watchdog—not return an early successful
process exit.

## Optional second seam, not part of an untested claim

To test a real present-time retry later, arm a one-shot removal from the View's
paint callback after a successful native begin (with its already-warm image),
then allow normal runtime command submission/present to proceed. This removes an
idle native device while a CPU frame exists; it is not a hardware-TDR test.
Do not replace present with a fake error. Exact direct observation of the
present Result would require an additional strictly forwarding Frame observer;
that is avoidable for the first begin-retry slice above. Root and child present-
retry variants can be a separate small follow-up if required by final acceptance.

## Expected RED-capability

The proposed native tests should fail if any of these are deliberately regressed:

- Root retry handling becomes fatal: native run returns nonzero before recovery.
- Child retry loses deferred scheduling: no fresh child frame, watchdog expires.
- Recovery clears retained CPU images/glyphs: pixels/reupload payload fail despite
  application upload counters remaining one.
- A peer retains the removed singleton: reconstruction fails while both windows
  remain alive.
- Lost-frame handling reports success or reuses stale capture: new epoch marker,
  generation and actual payload checks fail.

These are proposed mutation checks, not results already measured. Full native
Intel/NVIDIA/WARP/debug/GPU-validation/ASan execution and independent review remain
required once the fixture exists. No production source, test source or manifest
was changed while writing this note.
