# Experimental Direct3D 12 backend

DX12 is an **opt-in experimental backend** for opaque ordinary windows and
alpha-composited native Popups, not a complete replacement for Vulkan. Vulkan
remains the Windows default; Linux stays Vulkan and macOS stays Metal. The
previous Popup policy is unchanged for Vulkan.

## Implemented slice

- Native D3D12 hardware device (feature level 11.0 and shader model 6.0), direct
  command queue, root signature/PSO, two-buffer swapchain and fence-based
  resource retirement. The first suitable hardware adapter is selected; software
  fallback is not automatic.
- Ordinary windows retain an opaque HWND flip-discard swapchain. Native Popups
  use a premultiplied flip-sequential composition swapchain on the same DX12
  command queue, hosted privately by DirectComposition. There is no D3D11 device,
  shared texture/keyed mutex or extra presentation copy on this DX12 path, and
  no new application-facing window mode. The first successful presented and
  validated frame commits the visual; creation does not expose an unpainted frame.
- Clear, solid and rounded rectangles, selection/caret rectangles, clipping,
  composed transforms/opacity, DPI scaling and original draw order.
- Text uses the shared shaping/rasterization path and persistent R8 glyph atlas
  pages, including native DirectWrite glyphs. Images support RGBA8 uploads,
  nearest/linear clamp sampling, source crops, tint and cached reuse. Both consume
  the neutral CPU geometry and preserve order when interleaved with rectangles.
- Outer box shadows reuse the shared analytic shader and CPU geometry: CSS
  blur radius/sigma and spread, rounded corners, original-box clip-out, affine
  transforms, composed opacity, clips and DPI. Shadow commands retain their
  position in the same rectangle/text/image draw stream.
- Backdrop blur snapshots the scene immediately before each effect, reuses the
  shared downsample/separable Gaussian/rounded-mask shader, mixes tint and
  composed opacity, then resumes the original draw stream. `blur_radius` is
  Gaussian sigma (unlike outer-shadow CSS radius/2). The actual RGBA8
  render/load/filter-sample capability is queried from the device.
- Pixel readback with row-pitch handling, resize, and explicit rejection of
  overlapping frames or resize while a frame is outstanding. Destroying a
  renderer before its outstanding frame is safe; that frame cannot present.
- Shared CPU geometry in `renderer/planning` and shared Slang primitive shaders;
  no DX12-to-Vulkan package dependency or runtime shader compiler.
- Vulkan and DX12 populate `Renderer::backend_info()` and
  `Window::backend_info()` with the actual backend/adapter, vendor/LUID,
  software flag and selection. Other renderers, including the unchanged Metal
  backend, use the base `unknown` report until they opt in.

This is a synchronous baseline: GPU completion is awaited for each submitted
frame. Upload allocation pooling and deeper frame overlap remain optimization
work; the bootstrap is not a performance-parity claim.

Image uploads and invalidations update the CPU cache while commands are recorded;
abandoning or rejecting a frame does not roll back those cache operations. Each
recorded draw retains an immutable texture snapshot, so replacing/invalidation
later in the same frame does not change earlier draws. Uploads reach the GPU
lazily when first drawn; images are evicted after more than 120 successfully
presented idle frames. Glyph pages are cached for the renderer lifetime and
republished as full-page snapshots when dirty. This is correctness-first, not a
partial-upload, pooled-descriptor or bounded-glyph-cache performance guarantee.

Backdrop PSOs are lazy and cached. Each visible effect owns a full-frame scene
snapshot and two region-sized downsample targets with immutable descriptors.
They are retained until the next successful frame-start idle wait or renderer
destruction (or confirmed removed-device retirement), including across resize;
frames without effects allocate none.
This avoids reusing resource states from unsubmitted command lists, but is not
target pooling or a bounded peak-memory/performance guarantee. Full renderer
diagnostic snapshots/timings remain unimplemented; effect capability reporting
does not imply diagnostic parity.

Popup clears are premultiplied before ClearRenderTargetView; ordinary HWND clear
behavior is unchanged. Existing draw blending produces premultiplied target
pixels. On a composition surface, a dedicated backdrop shader replaces RGBA by
interpolating the original scene and filtered/tinted scene through the rounded
mask and effect opacity. It does not source-over the same scene alpha a second
time. This is still in-app backdrop blur, not sampling or blurring other desktop
windows behind the Popup. The existing opaque and Vulkan shader entries are
unchanged.

Native Popup swapchains remain premultiplied even when transparent intent is
false, as on the existing Vulkan composition path. `set_transparent_surface`
updates that intent/report without changing the HWND topology or actual alpha
mode. Ordinary HWND transparent creation and enabling transparency are still
rejected; they must not silently become composition windows or lose native menus.

Synchronization retains each successfully queued fence ticket across event or
wait failures. An event wake alone is not completion: the completed fence value
must reach that ticket. Destruction retains GPU ownership and retries with
backoff until completion or actual device removal is observed. Device removal
is never successful presentation; only a completed reconstruction can produce
a usable frame retry, as described below.

Private fault-injection tests cover repeated Signal failure, event/wait failure,
and premature event wake at the actual state-destructor/synchronization boundary.
A separate public-renderer test removes a real native idle device. These do not
certify arbitrary driver failure or TDR recovery.
If Signal permanently fails while the driver reports a live device and no
completion can be proven, destruction can block indefinitely. A timeout cannot
make releasing in-use GPU resources safe. The backend does not force device
removal (the native device may be shared by other renderers), leak the state as
normal recovery, or silently switch backends.

## Bounded device recovery

On confirmed native device removal, the renderer can rebuild once per public
operation on its original adapter LUID. HWND, framebuffer size/DPI and CPU image
and glyph caches survive. Native textures/descriptors/upload status now belong
to a private per-renderer realization cache, not immutable CPU draw snapshots;
native realizations can be retired during recovery even while finished frames or
invalidated image snapshots remain alive. Expired realizations are pruned after idle.

All old GPU owners are retired before replacement. `device_generation` advances
only after device, pipeline, swapchain and native validation succeed. A recovered
`begin_frame` or failed `present` returns `frame_retry_required` to request a new
frame, not claim the lost frame was presented. Failed frames do not advance image
cache idle ages. Resize instead completes the requested extent/DPI after recovery
and returns success, or returns an error; it never returns a frame retry.

Popup native retirement also detaches and releases the composition visual,
target and device before releasing the swapchain. Detachment/commit is best
effort during teardown; local references are released even on native errors.
There is no unbounded wait for compositor commits. If an external/compositor
owner still blocks reconstruction, the bounded attempt returns its actual error.
Successful reconstruction creates a new host with a fresh first-frame commit.

Recovery now coordinates renderers on the **same adapter and HWND-owning UI
thread**. Confirmed native loss first retires eligible peers' old GPU resources;
only the triggering renderer rebuilds immediately. Other peers remain pending
until their next begin/present/resize, and increment their own generation only
after their own complete reconstruction. An old active CPU frame requests a new
frame rather than submitting across generations. It remains outstanding until
presented or abandoned; old-frame destruction cannot unlock a newer frame.

A private weak registry holds no device reference. Its metadata lock is never
held across native calls. Per-renderer recursive operation gates protect state
access, and peer gates use nonblocking try-locks. Orphan states kept alive by old
frames can retire native resources but are not rebuilt for a dead renderer.
An operation-depth guard also skips same-thread reentrant active entries;
recursive lock acquisition alone is not treated as an idle-operation guarantee.
If the triggering renderer is itself inside a nested operation, confirmed or
pending recovery returns `ERROR_BUSY` before modifying recovery state or retiring
any native resources. Peer retirement is itself an active guarded operation.
Peers' CPU caches, capability reports and borrowed capture storage survive
native-only retirement. Pending peers report no current capture; an existing
borrowed capture remains alive until that peer's own next mutating operation.
This is not a thread-safe borrowed-pointer guarantee or permission to race
destruction with calls. Individual calls are not newly restricted to the creation
thread, but automatic peer coordination only runs on the HWND-owning thread.

External native references, different-window-thread peers, busy peer operations,
or a concurrently finishing native destructor may still block replacement of
the removed singleton. The attempt returns the actual terminal initialization
error with unchanged generation, not an unbounded retry loop. A skipped peer
must itself progress through recovery or release its native resources; simply
unlocking its operation gate does not trigger background recovery. Once blockers
are gone, an explicit later call can reconstruct the same renderer. The normal
UI runtime exits on terminal frame errors and does not recreate factories or
coordinate foreign threads automatically. Invalid input and live-device sync
errors are not treated as removal, and no backend/adapter fallback occurs.

Native acceptance uses explicit idle-device `RemoveDevice` and independently
checked post-recovery pixels. It does not certify hardware reset/TDR behavior.

## Selection and testing

On Windows, `create_renderer` reads `CGPUI_RENDERER`:

| Value | Behavior |
| --- | --- |
| Unset, empty, `auto` | Vulkan only until DX12 reaches parity |
| `vulkan` | Vulkan only; return its error on failure |
| `dx12` | Experimental DX12 only; return its error on failure |
| Anything else | `invalid_argument`; create neither backend |

Named factories in `cgpui/renderer/renderer_backend.hpp` bypass that selection:
`create_vulkan_renderer(descriptor)` and, on Windows,
`create_dx12_renderer(descriptor)`. Application code can supply either through
its existing renderer factory; `RenderSurfaceDescriptor` has no new backend or
native/composition mode field.

Windows applications using `create_renderer` link `cgpui.renderer-windows`,
which owns both backend dependencies. Backend-only consumers may link the
named backend package. SDK link closure includes the dispatcher and both
backends, but this does not change the default or certify a DX12 release.
The dispatcher still carries the normal Vulkan loader dependency: selecting
`dx12` does not provide loader-free startup. Vulkan delay-loading is not
implemented. Linking only the named DX12 package avoids its Vulkan package
dependency.

Run the native acceptance suite from PowerShell:

```powershell
$env:MCPP_JOBS = '1'
scripts/dev/test.ps1 -p tests/renderer-dx12
# Deterministic software rasterizer and strict native validation:
$env:CGPUI_DX12_ADAPTER = 'warp'
$env:CGPUI_DX12_DEBUG = '1'
scripts/dev/test.ps1 -Renderer dx12 -p tests/renderer-dx12
```

`CGPUI_DX12_ADAPTER` also accepts an explicit DXGI adapter index, with no
fallback. `CGPUI_DX12_GPU_VALIDATION=1` requests GPU-based validation. Debug
requests require the installed Windows graphics debugging layer; missing
support returns an error instead of silently claiming validation. Debug
configuration is fixed before the process's first DX12 device and cannot be
changed by later renderer creations. These environment variables are diagnostic
controls, not cross-platform window options. Use a separate process per debug
configuration. The hello-window and backdrop-blur examples can use the opaque
ordinary-window subset. Use dedicated native tests for pixel acceptance; one first-frame
smoke does not establish full workbench interaction or desktop appearance.

## Not implemented yet

Transparent ordinary windows/native-menu coexistence and coordinated
cross-thread device-loss recovery are future work.
Unsupported capabilities reject their commands rather than silently dropping
content. `popup_windows_supported()` is true for the implemented native Popup
path, including on an opaque parent renderer. The runtime still checks platform
support and actual Popup renderer initialization/placement. Genuine failures
permit the existing Overlay fallback; neither backend is preemptively disabled.

Popup native acceptance checks both captured RGBA and actual desktop pixels over
a controlled background, plus pointer-transparent/nonactivating platform windows,
resize/DPI updates, destruction and idle-device removal. This does not certify
every desktop configuration, physical monitor migration or arbitrary driver TDR.

The original DX12 plan's shared extraction and shader phases are only partially
complete: rectangle/text/image/shadow/backdrop algorithms and DXIL are implemented,
but full parity, transparent-window/native-menu compatibility, diagnostics, recovery,
performance gates and default-switch/release verification must precede automatic
DX12 selection. No release was published by this milestone.

See `.planning/2026-11-23-dx12-bootstrap/`,
`.planning/2026-11-23-dx12-textures/`,
`.planning/2026-11-23-dx12-shadows/`,
`.planning/2026-11-23-dx12-backdrop/`,
`.planning/2026-11-23-dx12-retirement/`,
`.planning/2026-11-23-dx12-recovery/`,
`.planning/2026-11-23-dx12-cohort/` and
`.planning/2026-11-23-dx12-popup/` for dated validation evidence.
