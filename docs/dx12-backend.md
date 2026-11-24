# Experimental Direct3D 12 backend

DX12 is an **opt-in experimental backend** for opaque ordinary windows,
bounded initially transparent ordinary windows and alpha-composited native
Popups, not a complete replacement for Vulkan. Vulkan
remains the Windows default; Linux stays Vulkan and macOS stays Metal. The
previous Popup policy is unchanged for Vulkan.

## Implemented slice

- Native D3D12 hardware device (feature level 11.0 and shader model 6.0), direct
  command queue, root signature/PSO, two-buffer swapchain and fence-based
  resource retirement. The first suitable hardware adapter is selected; software
  fallback is not automatic.
- Opaque ordinary windows retain an opaque HWND flip-discard swapchain. Native
  Popups and eligible initially transparent ordinary windows use a premultiplied
  flip-sequential composition swapchain on the same DX12
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
  render/load/filter-sample capability is queried from the device. When it is
  reported unavailable, the shared fallback draws only the requested tint.
- Pixel readback with row-pitch handling, resize, and explicit rejection of
  overlapping frames or resize while a frame is outstanding. Destroying a
  renderer before its outstanding frame is safe; an already-acquired unfinished
  frame can finish once while its original native epoch and caller-owned HWND
  remain valid. Recording and capture requests remain available on that frame.
  Ownerless frames never rebuild devices or advertise recovery/pending retries;
  removed/purged epochs, destroyed HWNDs and already-finished frames reject.
- Nonblocking DXGI frame admission for opaque HWND and composition chains,
  including the first frame. A pending begin returns `frame_not_ready`; WindowRuntime defers redraw
  through its window-bound timer rather than publishing success or hot-looping.
- Shared CPU geometry in `renderer/planning` and shared Slang primitive shaders;
  no DX12-to-Vulkan package dependency or runtime shader compiler.
- Vulkan and DX12 populate `Renderer::backend_info()` and
  `Window::backend_info()` with the actual backend/adapter, vendor/LUID,
  software flag and selection. Other renderers, including the unchanged Metal
  backend, use the base `unknown` report until they opt in.

This is a synchronous baseline: GPU completion is awaited for each submitted
frame. Geometry and texture staging use the separate bounded private upload
pools below; readback pooling and deeper frame overlap remain optimization work.
This is not a frame-rate or performance-parity claim.

Admission is separate from GPU retirement: it does not replace a fence wait or
make Present/readback asynchronous. Direct clients must schedule their own
bounded pending retry; fatal and recovery results keep their original meaning.
Both routes retain the same waitable flag across same-chain resize, without
reacquiring the app-owned handle. Native creation/configuration/wait errors are
not converted to flags-zero retries or unpaced success. Composition admission
does not require pre-Commit warmup: the visual still commits only after the first
successful, validated Present. This does not change Popup's native-first,
failure-only Overlay policy or expand ordinary-window alpha compatibility.
See [frame readiness](frame-readiness.md) for grant lifetime, runtime cancellation
and the native/controlled-test evidence boundaries.

An outstanding or finished frame retains its backend state independently of the
Renderer facade, including native presentation objects and any ordinary-alpha
window lease. The final state owner retires GPU work and composition before
releasing the alpha token. This does not keep the external platform window alive
or permit HWND lifetime races; alpha operations retain their owner-thread/token
checks. No new recovery or cross-thread coordination is promised for an orphan.
There is no Renderer getter after facade deletion: callers cannot inspect an
orphan capture by dereferencing the deleted facade or a stale borrowed pointer.

Image uploads copy caller bytes into a frame-owned pending batch. The final
upload for an ID supplies every image draw in that frame; pending invalidations
are applied first, irrespective of their recording order, then uploads and
image-draw resolution. Abandonment and rejection before image preparation do
not apply that batch. A failure after preparation starts does not promise
rollback; callers can resupply needed assets. Invalid intermediate same-ID
uploads may be replaced before final validation, but an invalid final upload
still rejects. DX12 continues to reject missing/incompatible image assets at
resolution; Vulkan missing-image behavior/error codes remain a separate policy
difference, not silently standardized here. Resolved draws retain immutable
CPU snapshots, including across later frames' replacements/invalidation.
Uploads reach the GPU lazily when first drawn; images are evicted after more
than 120 successfully presented idle frames. Glyph pages are cached for the renderer lifetime and
republished as full-page snapshots when dirty. This is correctness-first, not a
partial-upload, pooled-descriptor or bounded-glyph-cache performance guarantee.

Backdrop PSOs are lazy and cached. Each visible effect owns a full-frame scene
snapshot and two region-sized downsample targets with immutable descriptors.
They are retained until the next successful frame-start idle wait or renderer
destruction (or confirmed removed-device retirement), including across resize;
frames without effects allocate none.
This avoids reusing resource states from unsubmitted command lists, but is not
target pooling or a bounded peak-memory/performance guarantee. The bounded frame
diagnostics below do not imply performance or diagnostic parity with Vulkan.

If `backdrop_blur_support().supported` is false, the frame first validates the
device-scaled bounds, tint and clips, then delegates the original logical draw to
`RenderFrame::draw_backdrop_blur`. A present tint becomes an ordinary rounded
rectangle, retaining radius, clips, composition/metadata and draw order with one
DPI conversion; no tint is a no-op. This fallback does not perform blur or allocate
offscreen targets. It does not suppress invalid input, native API/presentation
errors or device-loss retry; supported blur still follows its normal effect path.

Fallback diagnostics describe the rounded work actually recorded, not a blur:
visible tint, including alpha/opacity zero, adds one rounded command/draw; no tint
adds none. Empty or fully clipped tint retains one planned, zero submitted
commands and zero draws. All fallback blur/effect/offscreen counts remain zero.
The focused `dx12_backdrop_blur_fallback_*` tests force only private capability
state after real native initialization; their captures and RemoveDevice checks
are not evidence from an incapable physical adapter. The existing capability
probe also reports failed `CheckFeatureSupport` queries as support=false with a
reason; this change does not distinguish that from missing format support.

Composition-surface clears are premultiplied before ClearRenderTargetView;
opaque ordinary HWND clear behavior is unchanged. Existing draw blending produces premultiplied target
pixels. On a composition surface, a dedicated backdrop shader replaces RGBA by
interpolating the original scene and filtered/tinted scene through the rounded
mask and effect opacity. It does not source-over the same scene alpha a second
time. This is still in-app backdrop blur, not sampling or blurring other desktop
windows behind the Popup. The existing opaque and Vulkan shader entries are
unchanged.

Initially alpha-composited swapchains remain premultiplied even when transparent
intent becomes false, as on the existing Vulkan Popup composition path.
`set_transparent_surface` updates that intent/report without changing the HWND
topology or actual alpha mode. An opaque ordinary renderer still rejects an
attempt to enable transparency; it never silently changes presentation routes.

## Bounded geometry upload reuse

Vertex and index data for rectangles/shadows, textured draws and backdrop
composites use private, persistently mapped 256 KiB upload pages. Each draw gets
its own 16-byte-aligned slice; the native buffer view uses that slice's GPU
address and actual byte length. Sharing storage does not merge draw commands,
change clipping/order or alter original diagnostic resource identities.

At most four ordinary pages (1 MiB of payload capacity) are retained for reuse
after the next safe frame begin. Additional pages and oversized allocations are
transient and are retired at that boundary. This caps retained arena capacity,
not a frame's peak allocation or the renderer's total memory. A large frame can
still require more upload storage until its commands are safe to retire.

The frame-start fence must prove completion before page cursors reset or
transient allocations retire. A signal/event/wait error or a wake without fence
completion does not authorize reuse. Native teardown clears arena resources
before releasing the device, including removed-device and cohort-peer cleanup;
old-device pages never enter a reconstructed device's arena. Persistent mapping
does not change these ownership or GPU-completion requirements.

The geometry adapter and the texture-staging adapter below share a private
low-level page allocator, not their page storage or slice policies. Geometry
still uses 16-byte-aligned slices with native UINT-sized views. Neither adapter
adds asynchronous frame overlap, draw batching or a new public configuration
API. Public upload-byte diagnostics still describe texture payload only, so
geometry-page reuse does not reduce or inflate that counter.

## Bounded texture staging reuse

Image and R8 glyph-atlas uploads use a separate pool of persistently mapped
256 KiB staging pages. A focused texture adapter applies the device-provided
copy footprint, 512-byte placement alignment and row pitch. Each native copy
references its slice offset within the staging page. Source bitmap padding is
not treated as image data, and successive slices must remain disjoint until the
recorded copies have completed.

This pool independently retains at most four ordinary pages after the next
proven-safe reset. Geometry and texture staging together therefore retain at
most 2 MiB of ordinary page payload capacity. Surplus and oversized allocations
remain alive until safe retirement; neither the cap nor the counter describes
peak frame memory, native texture allocations or all renderer memory.

Both pools reset only after the existing frame-start completion proof, never on
an unsuccessful wait or merely on public begin-frame. Native teardown clears
both pools before releasing the old device. The prior unstructured staging
resource list is replaced by the texture pool's owning lifetime boundary.

Only staging storage is reused: native image/glyph texture resources, immutable
CPU snapshots, lazy first use, duplicate-draw suppression, full dirty-glyph-page
uploads and recovery reuploads retain their existing behavior. Cache hits still
record no upload. Public upload-byte diagnostics count tightly packed texture
payloads, not row-pitch padding, placement gaps or page capacity. The low-level
allocator does not own texture formats, footprint calculations or GPU copy/draw
recording.

Readback buffers, descriptors and backdrop render targets are not pooled by this
change, and glyph updates are not partial. Fewer committed staging/geometry
allocations with unchanged pixels and lifetimes are the bounded claims, not an
FPS improvement, asynchronous renderer or performance equivalence to Vulkan.

## Frame diagnostics

DX12 retains the last successful requested capture on an unchanged valid surface
and device, matching Vulkan's capture history. Uncaptured successful frames,
abandoned requests and failed frames do not discard or replace that history;
before the first successful capture the getter returns null. A new successful
requested capture replaces it. This does not make retained pixels evidence of
the latest attempted or uncaptured frame.

Publication still follows readback, debug validation, required composition Commit
and successful diagnostic completion. Readback owns an unpublished CPU value until
that boundary: a late failure exposes no candidate pixels and preserves previous
successful pixels when the device/surface remains unchanged. Borrowed storage must
be copied before later renderer/frame operations or destruction. This contract is
for capture-capable built-in backends, not custom no-op implementations.

Capture history is not promised across resize, surface replacement or device
recovery. DX12's extent-changing ResizeBuffers path and recovery keep their
explicit invalidation; a recovery-pending peer's getter remains null. Same-size
resize does not newly clear pixels. These boundaries and native borrowed-buffer
lifetime rules are unchanged by the history alignment.

DX12 implements the existing `Renderer::last_frame_diagnostic_snapshot()` API;
there is no DX12-specific public diagnostic type. It returns null until the first
fully successful present. Publication follows submission, GPU completion,
optional pixel readback, debug-layer checks and composition commit. A failed or
abandoned frame never publishes its candidate snapshot. Resize and device
recovery preserve the last successful snapshot, even when it describes an older
device generation; it is not evidence that the current device has presented.
The borrowed result remains subject to renderer lifetime, the next successful
publication and the existing caller synchronization requirements. Copy the
snapshot when retaining historical measurements across later presents.

`retained_image_byte_count` is an optional CPU-payload sample taken after the
successful frame's image-cache aging/eviction and before snapshot publication.
DX12 counts each distinct live immutable RGBA image snapshot once across cache,
active/finished frames and submission references. Replacement or invalidation
does not subtract pixels still held by an earlier draw; releasing those owners
appears in a later successful sample. Recovery reupload does not create another
CPU copy. This is independent of per-frame upload bytes, which may be zero.

The count excludes caller-owned assets, row padding, glyphs, allocation capacity,
native textures/descriptors, unapplied raw frame-upload requests, staging/readback
and backdrop storage. Pending raw request buffers are destroyed on image
preparation exit, before successful sampling; finished frames retain resolved
immutable snapshots, not a second raw upload copy. It is not
RSS, VRAM, total renderer memory, a peak or a budget. Known zero differs from
unreported/unavailable: Vulkan, Metal and custom snapshots currently default to
no value. DX12 also reports no value if its weak bookkeeping registration cannot
allocate; it never publishes a known partial sum. This unavailability persists
for that image-cache lifetime, including across native recovery. Existing image
allocation and other diagnostic allocations retain their existing exception
behavior; this is not a general out-of-memory guarantee. Representational overflow
clamps to `SIZE_MAX` and sets the independent
`retained_image_byte_count_saturated` flag. Historical samples, including this
availability and saturation, are unchanged by failed/abandoned frames or resize.

Counts describe logical backend work, not GPU draw-call or pixel counts:

- Commands retain their original public draw-call identity and primitive kind,
  including selection and caret rectangles. Clear/upload/invalidation operations
  are not draw commands. One text command can produce several texture runs.
- A planned batch is a backend payload group with at least one eligible resource.
  Planned draws include only nonempty geometry ranges, textured runs and backdrop
  composites whose resolved scissor is visible. Fully culled commands remain in
  the planned command count, but create neither planned resources nor dropped
  resources; they can therefore appear as pending commands. Nonempty refers to
  native payload, not pixel coverage: a zero-area image quad still counts if its
  degenerate triangles and texture upload are actually recorded.
- Submitted draws are recorded only after their native draw command is emitted.
  Submitted commands and payload groups are counted once even when they emit
  several resources. A backdrop composite is one logical draw; its downsample,
  horizontal and vertical filtering contribute three offscreen passes instead.
  Offscreen-target allocation reports actual retained native resources.
- Upload bytes are the tightly packed texture payload of actual recorded upload
  copies, separated into R8 glyph-atlas and RGBA8 image bytes. Repeated draws of
  the same already prepared texture do not count another upload. Lazy first use,
  immutable replacement snapshots and post-recovery reuploads count when copied.
  CPU cache writes alone, row-pitch padding, vertex/index buffers, scene copies
  and capture/readback are excluded. Counts use saturating arithmetic.

Timings sum selected, nonoverlapping CPU wall-clock scopes: the successful
`begin_frame` admission call and presentation stages, including synchronous GPU
waiting and optional readback. They are not one begin-to-Present stopwatch.
`pacing_wait_nanoseconds` adds that one admission-call interval to the existing
pre-record synchronization interval; it is not an exclusive DXGI wait duration
or a measure of all synchronization. The post-Present fence wait remains in the
presentation stage. Admission timing includes owner/API overhead for both opaque
and composition chains; cached grants receive a fresh call sample without an
extra native wait. Zero is valid on a coarse clock and does not prove readiness.

Earlier pending probes, runtime timer/event-loop delays, time holding an abandoned
or active frame, recovery handling and application frame recording are excluded.
Each frame owns its own admission sample; failed/unpublished candidates never
replace the last successful diagnostic. Resource preparation includes diagnostic
planning. The frame-batched CPU image prefix runs before this diagnostic owner
and is excluded from these selected stage sums; a full Present API wall-clock
measurement includes it. All stage contributions use the shared saturating category/total sum.
These values do not measure GPU timestamps, scanout or end-to-end input latency.
Resource realization may occur during command recording, so stage names do not
promise identical work distribution across backends. Vulkan and DX12 have
different batching and upload policies; their raw counts are not a cross-backend
performance-equivalence claim.

## Bounded ordinary-window transparency

Initial `transparent_background=true` is supported for a CGPUI-owned, decorated
ordinary window with a visible **system or client-drawn title bar**, on Windows
build **26100 or newer**, and only if the real DWM alpha attribute succeeds.
System-title-bar windows support native application HMENU bars; client-title-bar
windows support this route **only without an attached application HMENU bar**.
The native system/window menu is separate and remains available. Use the normal
cross-platform descriptor intent, not a DComp/native window mode. Raw HWNDs,
hidden title bars, undecorated windows and older Windows remain unsupported.
Native Popup behavior is separate and unchanged. An active ordinary-alpha lease
fixes its initial title-bar mode: system/client switches, hiding the title bar,
removing decorations and fullscreen are rejected without changing the window.

The platform owns `DWMWA_REDIRECTIONBITMAP_ALPHA` and native-menu alpha repair;
DX12 owns its private composition host. A token-only, per-window-instance lease
connects them without linking DX12 to concrete platform-window objects. Native
HMENU rendering on the supported system-title-bar route remains Windows-owned:
after relevant native nonclient/menu
events, the platform copies the existing menu RGB through a nonclient buffered
paint and makes its alpha opaque. It does not redraw text, replace/hide the menu,
use undocumented UAH drawing hooks, or repair pixels in every render frame.

Client-title-bar alpha preserves `WS_CAPTION`, `WS_SYSMENU`, the existing
minimize/maximize styles and native controls; it does not use a captionless style
workaround. Initial alpha creation with an existing application menu returns
`unsupported_platform` without hiding or replacing the menu. While any window
in the application holds a client-alpha lease, a nonempty
`install_native_menu` request is rejected by a whole-registry preflight before
detaching menus or changing the model/accelerators. Empty clearing remains
allowed. During both install and clear transactions, a thread-local reservation
rejects nested installation and new client-alpha acquisition, including windows
temporarily without an HMENU. Raw out-of-band `SetMenu` is not supported: lease
status/repair detects it rather than claiming compatibility. The pre-existing
opaque client-title-bar/HMENU layout defect is not fixed by this slice, and no
framework-drawn replacement menu is installed.

This is **initial-only**, not an opaque-to-transparent upgrade. Opaque windows
keep their existing flip-discard and clear semantics. A recorded HWND-flip
attempt permanently disqualifies that HWND instance from later ordinary-alpha
creation, even after the opaque renderer is destroyed. A probe found stale
opaque client content after retiring the old flip chain; success of GPU teardown
alone was not sufficient. An all-composition-from-start experiment is research,
not the production default. See the [probe evidence](dx12-ordinary-transparency-probe.md).

The native alpha lease survives GPU recovery; native GPU objects retire before
the lease releases. Ordinary-alpha factory/begin/present/resize/intent operations
require the HWND-owning thread. Destruction on another thread queues only checked
identity tokens to the window thread. This restriction does not redefine the
existing opaque renderer's threading contract.

Initial **system-title-bar** native acceptance on 2026-11-23 exercised public
platform/renderer APIs:
changing red/blue GPU RGBA and real desktop alpha over a green backing window;
same-window opaque native menu references with exact whole-bar RGB/focus matches
before, while highlighted in a real menu loop, and after; genuine command and
Escape paths; late/replace/clear/reinstall menus; resize/renderer-scale changes;
duplicate creation, lifetime and two explicit idle-device removals with an opaque
peer. A separate Mica/none test reported actual Mica application and zero changed
menu pixels, then restored desktop alpha with a new blue frame. These are bounded
native tests, not physical mouse-hover, monitor-migration, all-driver or full
workbench acceptance. Older builds assert `unsupported_platform` and explicitly
skip only unsupported positive cases; that path has not been run on an old OS.

The client-title-bar acceptance uses the public factory without a preceding
chrome setter or a native-menu workaround. `dx12_client_initial_alpha_test`
checks initial frame geometry/styles and caption/control/resize hit tests,
premultiplied alpha 0/0.5/1 and desktop blending in both the title row and body.
`dx12_client_alpha_lifetime_test` covers fixed-mode rejection, resize and explicit
renderer scale 1/2, Mica/none, retained frames and window/renderer destruction;
`dx12_client_alpha_recovery_test` covers two explicit idle-device removals with
an opaque peer, capture lifetime, continued lease ownership and fresh pixels.
`dx12_client_menu_refusal_test` checks menu/factory refusal, unchanged commands
and application model, release/reacquire and the system-title-bar positive
control. `dx12_client_menu_reentrancy_test` checks actual resize callbacks during
install and clear with a menu-free target, plus nested installer refusal.
`dx12_client_controls_test` drives native maximize-button messages, minimize and
restore, close-request cancellation and initially nonresizable windows; it checks
GPU alpha while maximized and fresh desktop alpha after restore. These bounded
tests passed with WARP and the DX12 debug layer; renderer scale is not
physical monitor-DPI migration. See the scope decision and rejected menu-layout
probes in `.planning/2026-11-23-dx12-client-alpha/`.

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

Composition native retirement also detaches and releases the composition visual,
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
destruction with calls. Existing opaque/Popup calls are not newly restricted to
the creation thread, but automatic peer coordination only runs on the HWND-owning
thread. The new ordinary-alpha lease has the owning-thread restriction above.

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

The native `dx12_runtime_root_recovery_test` and
`dx12_runtime_child_recovery_test` also run through the real `WindowRuntime`
event loop, with root-first and child-first recovery on two same-thread windows.
The fixture observes the actual typed begin-frame retry without retrying the
frame itself or replacing the renderer factory. It checks the same adapter,
advanced generation, newly painted pixels, retained image/glyph reuploads and
the next warm frame's zero upload payload. Both tests passed under WARP with
the DX12 debug layer on 2026-11-24; no production runtime change was required.
This closes that bounded runtime-retry test gap, not foreign-thread recovery,
arbitrary driver resets, hardware acceptance or the final full regression matrix.

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
On Windows, neither the dispatcher nor the installed SDK imports `vulkan-1.dll`
at process startup. Selecting DX12 does not load Vulkan, even though both backend
libraries remain in the SDK link closure. Only Vulkan initialization loads the
DLL, through a private `LoadLibraryExW` owner using
`LOAD_LIBRARY_SEARCH_DEFAULT_DIRS`: the application directory, System32 and
explicit user directories registered with `AddDllDirectory`. It neither searches
the current directory/ambient `PATH` nor changes the process DLL-search policy.

The loader resolves all **88 required exports** into a typed candidate table
before publishing any function pointer. Existing optional version and
device-extension queries retain their previous checks. Missing or damaged DLLs
return `renderer_initialization_failed` with `vulkan-1.dll` and the Win32 error
code; missing required exports return the same typed error naming the export.
There is no backend fallback: default/`auto` still means Vulkan, and a failed
Vulkan initialization does not select DX12. Failed binding attempts release
their module reference and can be retried; successful binding is serialized and
keeps its module reference for process lifetime, including late resource teardown.

Building the repository on Windows still needs the Vulkan SDK headers.
Consumers of a newly built installed Windows SDK need neither the Vulkan SDK nor
its import library or delay-load flags/hooks; they need a Vulkan runtime only
when selecting Vulkan. Older static-import SDKs keep their manifest-declared
dependencies. Linux retains its original direct Vulkan linkage. This is startup
dependency isolation, not a DX12 default switch or a new release.

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

### Common Windows pixel contracts

`tests/renderer` contains 13 shared real-window scenes: backdrop blur, box
shadow, clipping, color encoding, frame capture, images, opacity, resize,
rounded rectangles, DPI scale, text, text ink extent and transforms. Their
`renderer_*` sources use one test-only native creation helper; the same scene
and pixel oracle runs against the backend explicitly requested by
`-Renderer vulkan` or `-Renderer dx12`; explicit `both` runs them in that order:

```powershell
scripts/dev/test.ps1 -Renderer both -p tests/renderer
scripts/dev/test.ps1 -Renderer vulkan -p tests/renderer
scripts/dev/test.ps1 -Renderer dx12 -p tests/renderer
scripts/dev/test.ps1 -Renderer dx12 -p tests/renderer renderer_pixel_backend_identity_test
```

`both` is a runner option, never a renderer environment value. Each backend gets
the same forwarded arguments; both invocations run after an ordinary test failure,
and the first nonzero exit code wins. A terminating setup/invocation exception
remains fail-fast. No selector keeps one invocation with the inherited environment;
explicit `auto`, `vulkan` and `dx12` also keep one invocation. ASan preparation and
member-path translation happen once, and caller environment/location are restored.
The Windows CI Python fixtures exercise orchestration with recording mocks under
PowerShell 7 and Windows PowerShell 5; they are not real renderer or ASan runs.

The helper calls the matching named factory, then requires the actual
`backend_info().backend` and selection to match, with no fallback reason.
It logs the requested/actual backend, adapter name, vendor, LUID and software
flag. A DX12 WARP request additionally requires a software adapter. Invalid
selectors, initialization failures, unsupported required capabilities,
missing captures or identity mismatches fail rather than silently skip.
Unset/empty/`auto` retains the common tests' existing named-Vulkan default;
production dispatcher/default behavior is unchanged.

Previously these scenes used a hard-coded Vulkan helper, so setting
`CGPUI_RENDERER=dx12` alone could not demonstrate DX12 coverage. The common
helper now reads the selector; it also owns the three formerly direct Vulkan
creations in backdrop/frame-capture/resize tests. Backend-private `vulkan_*`
tests still call Vulkan and are not DX12 evidence merely because the enclosing
suite was launched with a DX12 selector.

The focused `vulkan_backdrop_pass_diagnostics_test` and
`dx12_backdrop_pass_diagnostics_test` deliberately use named factories and
require exact offscreen pass counts **0/5/9** and **0/3/6**, respectively, for
zero/one/two effects. These backend-specific work counts are not equated; the
common backdrop scene retains its pixel/order/tint/scale and semantic-count
checks. Mathematical shadow references remain shared with Metal without
changing its native tests.

The common scenes are Windows HWND tests, not new Linux/macOS pixel coverage.
Their registration and strict selection are source facts, not a full-parity,
all-adapter or performance acceptance claim. WARP/debug/GPU-validation controls
above can be used for separate requested-backend runs; ordinary-window/menu,
recovery, example/UI, CI and release-readiness gates remain independent.

## Not implemented yet

Dynamic opaque-to-transparent upgrades, client-title-bar ordinary alpha with an
application HMENU, ordinary alpha on hidden/undecorated chrome or older Windows,
and coordinated cross-thread device-loss recovery are
future work. The [ordinary transparency probe](dx12-ordinary-transparency-probe.md)
retains the earlier 11 uncorrected DWM/hosting recipes and the later alpha-only
menu repair evidence; the negative recipes are not a claim that all DComp/native
menu combinations are impossible.
Unsupported window routes remain explicit errors; backdrop blur instead follows
the documented tint-only fallback above. `popup_windows_supported()` is true for
the implemented native Popup
path, including on an opaque parent renderer. The runtime still checks platform
support and actual Popup renderer initialization/placement. Genuine failures
permit the existing Overlay fallback; neither backend is preemptively disabled.

Popup native acceptance checks both captured RGBA and actual desktop pixels over
a controlled background, plus pointer-transparent/nonactivating platform windows,
resize/DPI updates, destruction and idle-device removal. This does not certify
every desktop configuration, physical monitor migration or arbitrary driver TDR.

The original DX12 plan's shared extraction and shader phases are only partially
complete: rectangle/text/image/shadow/backdrop algorithms and DXIL are implemented,
but full parity, broader transparent-window/native-menu compatibility, diagnostic/recovery parity,
performance gates and default-switch/release verification must precede automatic
DX12 selection. No release was published by this milestone.

See `.planning/2026-11-23-dx12-bootstrap/`,
`.planning/2026-11-23-dx12-textures/`,
`.planning/2026-11-23-dx12-shadows/`,
`.planning/2026-11-23-dx12-backdrop/`,
`.planning/2026-11-23-dx12-retirement/`,
`.planning/2026-11-23-dx12-recovery/`,
`.planning/2026-11-23-dx12-cohort/` and
`.planning/2026-11-23-dx12-popup/` and
`.planning/2026-11-23-dx12-ordinary/` and
`.planning/2026-11-23-dx12-nc-alpha/` for dated validation evidence.
