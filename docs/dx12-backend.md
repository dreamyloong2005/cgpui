# Direct3D 12 backend

Windows automatic selection prefers **DX12**, with Vulkan fallback only after
a classified unavailable-backend initialization failure and proven cleanup on the
same managed window. Explicit backend selection never falls back, and frame,
resize or recovery failures never switch APIs. DX12 supports opaque ordinary
windows, bounded ordinary-window transparency and alpha-composited native Popups;
its capability limits below still apply. Linux stays Vulkan and macOS stays
Metal. Popup policy remains native-first with Overlay only on genuine failure.

> Development status: this local implementation activates automatic DX12-first
> selection alongside bounded owned DRED diagnostics, owner-dispatched recovery,
> Runtime handling for [unfinished recovery](renderer-recovery-pending.md),
> [manual WARP CI](dx12-warp-ci.md), and bounded dynamic ordinary-window alpha.
> Native GPU-preference selection, explicit Vulkan device selection and Vulkan
> device-loss reconstruction are implemented. The attempt-7 snapshot passed the
> bounded Windows, hardware/WARP and local SDK checks below; remaining acceptance
> gates are stated separately. This is not a published SDK release, hosted CI
> result, or all-driver certification.

## Current validation status (2026-11-25)

The implementation snapshot in `root-attempt-7` has these fresh local results:

- Windows full regression passed **1028/1028**, and the optimized Release build
  completed all **39 workspace members**.
- Intel hardware 0, NVIDIA hardware 1 and WARP each passed the **243-test DX12
  suite** and **124 common tests per backend** (Vulkan/DX12). DX12 debug was
  requested; WARP also requested GPU validation. These are suite/lane results,
  not universal per-test adapter attestation: some initialization tests remove
  pins, and DX12 adapter settings do not select Vulkan devices.
- The local installed Windows SDK check covered the exact **129 DX12 source
  objects** and two healthy automatic-DX12 captures, in a clean environment and
  with an invalid app-local loader; Vulkan stayed unloaded. `/MT` was used; CMake was
  configure-only. It is not a published SDK or every-consumer/CRT certification.
- Local CI fixtures passed **38/38**; driver fixtures passed **17/17** on PS7
  and **15 with the two specified PS7-only skips** on PS5. SDK graph/header
  fixtures passed **9/9** and **15/15** without skips, and the runner fixture
  passed its three controlled cases. These are not hosted CI execution.
- The source-equivalent typed WSL export passed **725/725**, built all **39
  members**, and passed Linux GCC **2/2** and macOS architecture-rejection **1/1**
  fixtures. Source checks matched before and after; the four restored GNU tool
  symlinks preserve source content. This is not native macOS/Metal execution.
- Current-source ASan passed **243/243 DX12 tests** and **124/124 common tests
  per backend**, with zero skips. This is not per-test adapter attestation.
- A fresh performance run completed **18 series** on the same Intel adapter:
  three scenes (`rect-grid`, `cached-images`, `mixed`) x two backends x three
  repeats, each with **120 warmup frames and 600 samples**. All runs and raw-output
  audits passed. This establishes comparable recorded samples, not an FPS,
  GPU-time, speedup or performance-parity claim. The earlier visibility-guard
  failure remains a separate historical failure with **0 measured samples**.

Native Vulkan recovery tests inject failure results into real native calls after
safely draining a healthy device; they are not driver removal/TDR evidence. Native
GPU-preference tests check hardware ordering or explicitly proven unavailability,
not silent substitution with WARP/Vulkan. Broader desktop/driver and older-Windows
coverage, native macOS/Metal, hosted CI and release verification are not claimed.
No commit, push, publication or release is implied.

The Windows DX12 implementation milestone is complete within the supported
contracts documented here. The project owner deferred the original plan's
[native macOS shared-renderer check](macos-validation.md#follow-up-shared-renderer-after-dx12-extraction-2026-11-25)
as a separate macOS follow-up on 2026-11-25; it remains unverified, not waived
into a passing result.

## Implemented capabilities

- Native D3D12 hardware device (feature level 11.0 and shader model 6.0), direct
  command queue, root signature/PSO, two-buffer swapchain and fence-based
  resource retirement. Unset adapter selection chooses the first suitable hardware
  adapter; diagnostic selectors can request a native GPU preference, exact DXGI
  index or WARP as documented below. Software fallback is not automatic.
- Opaque ordinary windows use either the original HWND flip-discard route or,
  for eligible fresh managed system-title-bar or menu-free client-title-bar
  windows, a composition-from-start
  swapchain with **IGNORE** alpha. The latter preserves opaque clear semantics
  while enabling the bounded dynamic transition described below. Native
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
still rejects. Legal image draws whose pixels are unavailable are skipped, in
both DX12 and Vulkan, without rejecting the remaining frame. Invalid parameters,
identity mismatches and native failures still report errors. Resolved draws retain immutable
CPU snapshots, including across later frames' replacements/invalidation.
DX12 uploads reach the GPU lazily when first drawn; images are evicted after more
than 120 successfully presented idle frames. Glyph pages are cached for the renderer lifetime and
republished as full immutable CPU snapshots when dirty. Eligible native successors
use the conditional dirty-region transport below; CPU history and snapshots are
not bounded or eliminated by that optimization. Native SRV storage
uses the separate private descriptor pool described below.

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
topology or actual alpha mode on those initial-alpha routes. An ordinary HWND
flip renderer still rejects enabling transparency. The new eligible
composition-from-start route can replace its composition swapchain's alpha mode
without converting an HWND-flip window. Its bounded regressions are included in
the attempt-7 native suite; unsupported window topologies remain unsupported.

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
an unsuccessful wait or merely on public begin-frame. Texture staging is also
retained until its referring parent command list is successfully reset or released.
Native teardown clears
both pools before releasing the old device. The prior unstructured staging
resource list is replaced by the texture pool's owning lifetime boundary.

Staging storage reuse is separate from native texture realization and the glyph
transport policy below. Immutable CPU snapshots, lazy first use, duplicate-draw
suppression, full image uploads and recovery initialization remain. Cache hits still
record no upload. Public upload-byte diagnostics count tightly packed texture
payloads, not row-pitch padding, placement gaps or page capacity. The low-level
allocator does not own texture formats, footprint calculations or GPU copy/draw
recording.

Readback buffers and backdrop render targets are not pooled by this staging
change. Fewer committed staging/geometry
allocations with unchanged pixels and lifetimes are the bounded claims, not an
FPS improvement, asynchronous renderer or performance equivalence to Vulkan.

## Native SRV descriptor reuse

Each renderer's native state owns a private pool of stable, shader-visible
4096-slot SRV pages. Image and glyph native realizations hold move-only one-slot
leases; each backdrop group acquires three slots atomically from one page.
The latter matters for alpha composition, which consumes both original and
filtered textures in the same draw. Groups may use nonadjacent slots, but never
span heaps. Sampler and RTV storage and explicit per-draw table bindings are
unchanged; sharing a heap does not merge draws or eliminate those bindings.

Descriptors remain immutable while their native owner is retained. CPU image
and glyph snapshots do not hold native leases. Existing GPU-completion and
command-reference retirement boundaries permit expired native realizations and
prior backdrop groups to release their slots; glyph command dependencies retain
their leases through successful parent Reset/release as described below.
An unsuccessful wait or Present alone does not grant retirement. Lease
return allocates no memory. Native retirement releases all consumers before
clearing the pool and releasing the device. Recovery rebuilds native descriptors
on the new epoch while retaining the existing CPU cache semantics.

When pages lack enough free slots, allocation grows without moving existing
handles or splitting a blur group. Allocation failure preserves unrelated live
leases and propagates the error. Empty high-water pages remain until native
retirement: 4096 is the page size, not a total-resource or memory limit. This
changes descriptor allocation/storage, not glyph dirty-region upload policy,
frame overlap, public window options, backend defaults or Popup fallback.
Fewer live SRV heaps are not proof of better FPS, GPU time or lower peak memory;
use the separately documented steady-state probe for comparable measurements.

## Conditional glyph dirty-region transport

Each full immutable R8 CPU snapshot has private weak-identity metadata sealing
its page family and page-local upload prefix. Native state keeps at most one
completed resource-only base per current family, within its device epoch. These
sidecars do not pin CPU snapshot bytes. Unsubmitted or same-list candidates are
never treated as completed bases.

For a compatible prior-completed base and a validated nonempty suffix whose
tightly packed dirty payload D is smaller than the full page P, DX12 allocates a
distinct destination, clones P on the GPU, then uploads the exact dirty regions D
using the existing staging pool. This always-copy-on-write path preserves both
held and released predecessor snapshots. Exact native hits record no transport.
Cold, recovery, missing optional lineage, incompatible/backward bases and valid
D>=P use full initialization. Invalid source records or inconsistent sealed
metadata are errors, not disguised full-upload fallbacks.

Every one-record shared-planner invocation uses fresh production-resource state;
page-local progress is not inferred from that temporary planner's upload count.
Pending markers, node pins, candidates and target staging are prepared before
the target's first recorded reference. GPU completion publishes the greatest
completed prefix but does not release recorded dependencies. Successful parent
command-list Reset/release is a separate retirement condition; failed waits or
Reset retain ownership. Expired pinned map nodes move, with their sole SRV lease,
into preallocated detached slots until safe release. No raw-address identity or
strong CPU snapshot pin replaces the weak snapshot identity.

The optimization reduces eligible host-to-GPU payload, not total work: full CPU
snapshots and original glyph history remain, shared planning owns suffix payload
copies, native row alignment remains, and GPU cloneP adds work and a distinct
texture. The scoped work-observer tests are correctness/accounting probes, not
whole-renderer memory measurements or benchmarks. No FPS, latency, all-path
minimal-upload, cache bound or asynchronous-rendering claim follows from D<P.

## Conditional native command recording reuse

DX12 can reuse one immutable native BUNDLE for a restricted exact-static scene
on an opaque ordinary HWND. The renderer selects this privately; applications
choose neither a reuse mode nor another window type. This optimization does
not change transparent ordinary windows, native Popups or their existing
composition/fallback policies.

Eligible frames contain only rectangle-family geometry and already-uploaded
immutable RGBA images, with one common effective scissor and one SRV page.
Selection/caret rectangle geometry is included; glyphs, shadows and backdrop
blur are not. Ordinary paint opacity and image alpha on an opaque HWND do not
by themselves exclude reuse. Transparent/composition surfaces do.

Public capture requests, any image upload/invalidation request (including an
undrawn ID), missing/cold images, empty or culled work, differing scissors/pages
and other payloads use the ordinary DIRECT path. Private admission limits are
256 draw groups, 512 native draws and four 256 KiB frozen geometry pages, with
no oversized slice. Exceeding them is a cache miss, not a new rendering limit.
Input validation and native errors retain their original errors; they are not
converted into successful cache misses or dropped work.

The first eligible frame records and closes a BUNDLE. Later matching frames
execute that same native recording only after the previous submission completed,
passed all publication gates, and its referring DIRECT parent was successfully
reset. Equality includes device/surface generation, extent/scale, clear color,
ordered geometry/ranges and resource metadata, immutable image identity and
native SRV/sampler/pipeline bindings. Same-ID replacement is not the same image
snapshot. Changed inputs produce fresh recording or ordinary DIRECT work.

Every frame still records its DIRECT shell: the current backbuffer, clear,
viewport/scissor, barriers, submission and Present remain per-frame work.
BUNDLE geometry owns separate frozen storage rather than borrowing the rewound
transient arena. Strong immutable-image pins protect existing native map nodes
and their sole SRV leases; there is no second descriptor allocator. GPU completion
alone does not release the recording's dependencies: successful parent Reset or
actual parent release is also required. Unknown completion retains owners;
confirmed removal retires old native owners before reconstruction. A failed
attempt cannot reuse an earlier attempt's successful publication.

Successful hits retain current diagnostic receipts, image-cache aging and
capture-history semantics. Public capture deliberately records DIRECT work;
retained public pixels are not proof of a later uncaptured hit. The focused
correctness fixture instead observes the selected production target with a
separate same-queue readback submission before the covering fence. That fixture
uses two queue submissions and is not the normal rendering path or a timing
benchmark.

This reduces repeated native draw recording only for the stated subset. It does
not reuse the whole frame, add asynchronous overlap, establish total-memory
bounds, demonstrate an FPS/latency improvement, or qualify DX12 for automatic
selection or release. Frozen geometry is additional cache storage; its private
page cap does not bound native command allocators, descriptors or the renderer.

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

Cross-backend capture history is not promised across resize, surface replacement
or device recovery. DX12's extent-changing ResizeBuffers path and recovery keep their
explicit invalidation; a recovery-pending peer's getter remains null. Same-size
resize does not newly clear pixels. These boundaries and native borrowed-buffer
lifetime rules are unchanged by the history alignment. Vulkan's new device-loss
path retains the most recent successful CPU capture through native retirement,
including failed reconstruction; it does not publish a failed candidate or make
those historical pixels evidence of the new generation.

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
unreported/unavailable: Metal and custom snapshots default to no value. Vulkan
now samples retained recovery-upload RGBA payloads after successful presentation,
not DX12's cross-frame immutable-snapshot census; the two ownership models are
not identical memory measurements. DX12 reports no value if its weak bookkeeping registration cannot
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
  Eligible glyph successors count dirty host uploads D, not their GPU cloneP.
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

## Owned DRED status and best-effort breadcrumbs

When debug is requested, setup queries the native DRED settings interface before
device creation and records whether the breadcrumb-enable setter returned. This
is a request observation, not a claim that every external native device was
created with those settings. A later setup failure does not suppress the removal
interface query.

Before native retirement, a focused private observer reads the **actual**
`GetDeviceRemovedReason`. A failed native lease publishes one owned scalar
snapshot containing that HRESULT and the setup/interface-query facts. A live
device does not consume the observation opportunity; a later lease may publish
another snapshot while retaining prior history. Temporary QI references end
before return. Debug-off uses typed not-requested states and emits no new debug
message. Formatting uses fixed storage and makes unknown quantities explicit.

Debug-requested confirmed loss also queries `GetAutoBreadcrumbsOutput`. The
query HRESULT is distinct from QI status. A successful query is sampled through
`ReadProcessMemory(GetCurrentProcess(), ...)` into fixed owned storage, never
by directly dereferencing native node, name, history or COM-child pointers.
Every read must succeed with its exact requested byte count. An unreadable or
short read discards the current node; prior accepted nodes remain available.

Sampling is bounded to 64 nodes, 32 operations per node, 512 reads and 65536
requested bytes. History windows surround the last-completed count and respect
DRED's 65536-operation ring; native counts never determine allocation sizes.
Cycle, truncation, invalid address, changed metadata/progress, exhausted budget
and failed/short reads have distinct outcomes. Failed queries are not reported
as a successfully observed empty list. Bounded numeric samples go to debugger
messages, while the existing lease history retains only their scalar summary.

These are **best-effort, non-atomic, non-coherent diagnostics**, not a device-wide
snapshot. Equal metadata re-reads detect some changes but cannot exclude address
reuse or plausible unrelated values under concurrent external-owner mutation.
Samples never control recovery, native retirement or rendering results. No
borrowed addresses, names or COM references escape the observation. Per-lease
observation is not exact process-wide capture-once, and page-fault output is not
requested. Its bounded storage/call count is not a hard latency bound
on OS/debugger calls. Reporting cannot authorize native retirement or overwrite
the separate synchronization/removal proof. The attempt-7 suite includes these
bounded diagnostics tests; its scope and remaining gates are recorded in
[validation status](#current-validation-status-2026-11-25).

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

The original initially-alpha path retains its sticky premultiplied semantics.
The current implementation batch additionally reserves a private composition
route for a fresh, eligible managed **system-title-bar**, or **client-title-bar
without HMENU**, opaque ordinary window
created on its owner thread. It begins with actual `DXGI_ALPHA_MODE_IGNORE`, not
premultiplied clear/blending masquerading as opaque. Native HMENU is neither
hidden nor replaced. Unmanaged, old-OS, incompatible-chrome, prior-flip and
foreign-thread factory paths retain the original opaque HWND route.

The reservation alone does not enable native alpha repair or lock ordinary
chrome/menu changes. A later transparent request rechecks current eligibility,
including the client-title-bar menu and in-progress menu-installation gates,
constructs a complete replacement chain and pacing state, and binds it to the
same private composition host. Switching back uses IGNORE alpha; initial-alpha
windows and Popups keep their older sticky-PREMULT contract. Consecutive changes
before a successful frame coalesce without unbounded chain history. The original
visible chain remains owned until the successful frame's Commit, before capture
or diagnostics publication. Failed binding/rollback retains every possibly
bound owner and reports failure rather than publishing a fictitious transition.
Replacement is rejected with an outstanding frame or inside a nested operation.

Before the first alpha promotion, opaque worker operations remain supported while
the window owner pumps the bounded token-status handshake. Actual alpha changes
require the owner thread. After promotion the native alpha lease remains active
even when the composition chain switches back to IGNORE; therefore its existing
owner-thread, fixed-chrome and native-menu repair rules continue until renderer
state destruction. IGNORE still presents opaque pixels; it is not a released
alpha lease or restored worker/chrome permission.

A recorded HWND-flip attempt still permanently disqualifies that HWND instance
from later ordinary-alpha creation, even after renderer destruction. The stale
client-content failure in the [probe evidence](dx12-ordinary-transparency-probe.md)
is not bypassed by teardown or by clearing history. The new path avoids creating
that original flip chain; it does not attempt a proven-failing flip-to-composition
conversion. Dynamic GPU/desktop/menu/worker/lifetime/failure regressions passed
again in the bounded attempt-7 native suite.

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

On confirmed native device removal, DX12 can rebuild once per public
operation on its original adapter LUID. HWND, framebuffer size/DPI and CPU image
and glyph caches survive. Native textures/descriptors/upload status now belong
to a private per-renderer realization cache, not immutable CPU draw snapshots;
native realizations can be retired during recovery even while finished frames or
invalidated image snapshots remain alive. Expired realizations are pruned after idle.
Recovery looks up that recorded LUID, not a new adapter index or preference order,
and never rereads `CGPUI_DX12_ADAPTER`; changing the environment cannot move an
existing renderer to another adapter or backend.

Known local old GPU owners are retired before replacement; external references
are not owned by this protocol. `device_generation` advances
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

The new implementation adds **owner-dispatched coordination across managed
window threads** on the same adapter, retaining the same-owner idle fast path.
A private native lease starts before device creation, covering partial/in-flight
construction. Retirement is acknowledged only after actual native release, not
weak-state expiration. A loss wave blocks new local attachments until its known
participants are retired or independently observed healthy. The triggering
renderer returns `renderer_recovery_pending` while coordination is unfinished,
without waiting for a foreign thread under its operation gate. Other peers
rebuild only on their next legal begin/present/resize, advancing generation after
complete reconstruction. An old active CPU frame requests a new frame rather
than submitting across generations; its lifetime cannot unlock a newer frame.

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
destruction with calls. Native retirement callbacks execute on each managed
HWND's owner thread, using instance-bound integer messages, nonblocking state
gates and actual removal checks. Busy callbacks do not acknowledge or purge;
the outer operation may post a coalesced retry after releasing its gate.
Ordinary-alpha mutations retain their owning-thread restriction.

Each cooperation wave has a fixed five-second deadline. Missing managed owner
endpoints, failed native posts or an unpumped/busy owner past that deadline return
an explicit terminal error, not an unbounded retry or permission to free active
resources. External native references can still block reconstruction and retain
their actual native error. Once real release removes blockers, a direct client's
later explicit call may reconstruct. WindowRuntime defers pending recovery on
its window-bound timer and reapplies the latest desired resize before drawing;
terminal errors remain fatal. Invalid input and live-device sync errors are not
treated as removal; no backend/adapter fallback occurs. See the full
[pending and ownership contract](renderer-recovery-pending.md). Bounded managed
cross-thread tests passed again in the attempt-7 native suite; this does not
certify arbitrary external owners or driver resets.

Native acceptance uses explicit idle-device `RemoveDevice` and independently
checked post-recovery pixels. It does not certify hardware reset/TDR behavior.

The native `dx12_runtime_root_recovery_test` and
`dx12_runtime_child_recovery_test` also run through the real `WindowRuntime`
event loop, with root-first and child-first recovery on two same-thread windows.
The fixture observes the actual typed begin-frame retry without retrying the
frame itself or replacing the renderer factory. It checks the same adapter,
advanced generation, newly painted pixels, retained image/glyph reuploads and
the next warm frame's zero upload payload. Both tests passed again in the
attempt-7 native suite. This covers bounded same-thread runtime retry, not
arbitrary driver resets or general hardware acceptance.

### Vulkan native device-loss reconstruction

Vulkan now preserves raw `VkResult` at pacing, acquire, reset-fence/submit,
present, resize-idle and capture wait/map boundaries. Only confirmed
`VK_ERROR_DEVICE_LOST` authorizes device reconstruction; error-message text,
out-of-memory, timeout and out-of-date/suboptimal surface status do not. Failure
cleanup also preserves a device loss discovered while waiting to retire work.
Swapchain recreation and non-loss failures retain their separate error paths.

One synchronous rebuild retires the old device's native resources, including
private Popup interop owners, while preserving the instance, surface and logical
descriptor. Automatic Vulkan selection prefers the prior physical adapter if
still suitable; otherwise it may choose another suitable adapter. With an
explicit `CGPUI_VULKAN_DEVICE` index, recovery instead pins the originally chosen
`VkPhysicalDevice` identity within the preserved instance: changed enumeration
order or environment values cannot move it, and a missing/unsuitable original
device fails. Neither policy is an API/backend fallback. Generation increments
only after complete device resources are installed. Recovery during presentation
returns `frame_retry_required`, never successful presentation of the lost frame. Failed
reconstruction cleans its partial native owners and leaves a stable terminal
error; repeated calls do not restart it. The path does not emit
`renderer_recovery_pending` or add cross-thread owner coordination.

The CPU glyph cache survives; cleared native upload prefixes force restoration.
Accepted image-preparation payloads move from the frame into a retained CPU cache
without another RGBA copy. The latest accepted same-ID upload replaces its prior
payload, and invalidation, allocation changes and normal idle eviction prune the
retained entries. Failed frames are not transactional rollbacks of preparation.
Restoration preserves each image's prior idle age and allocation identity, lets
incoming uploads supersede old bytes, and checks the replacement device's current
texture limits before creating any retained image, including on an image-free
frame. Invalid identity/limit requests remain errors, not ignored images.
Vulkan keeps its existing preparation-frame cache-aging semantics; this is not
DX12's successfully-presented-frame aging policy.

Vulkan frames are single-use once a current-generation `present` starts, whether
that attempt succeeds or fails. A second present rejects rather than replaying
moved upload buffers. A frame acquired before device reconstruction returns a
typed retry without applying its uploads or invalidations to the new generation.
Acquire a new frame after retry. If Vulkan resize discovers loss, reconstruction
uses the requested size/scale and resize returns success only after confirmed
complete reconstruction and a generation advance. Otherwise it returns an error;
it does not return a completed-frame retry. This preserves WindowRuntime's shared
resize contract. The latest successful CPU capture survives this native retirement.

The new `vulkan_device_recovery_*` tests exercise native owners, retained
image/glyph pixels, stale frames, terminal reconstruction, non-loss controls,
smaller replacement limits and private Popup interop. Their loss hooks first prove
the real healthy device idle, then inject the selected native failure result;
they do not remove the driver device or mutate production renderer state. Their
smaller-limit hook is also a controlled capability simulation, not a physical
adapter migration. These tests passed in the attempt-7 Windows and common suites.

## Selection and testing

On Windows, `create_renderer` reads `CGPUI_RENDERER`:

| Value | Behavior |
| --- | --- |
| Unset, empty, `auto` | DX12 first; Vulkan only after classified failure and checked same-window rollback |
| `vulkan` | Vulkan only; return its error on failure |
| `dx12` | DX12 only; return its error on failure |
| Anything else | `invalid_argument`; create neither backend |

Automatic fallback is an initialization-only transaction, not an availability
promise for every failed DX12 setup. Only native `DXGI_ERROR_UNSUPPORTED`,
`DXGI_ERROR_NOT_FOUND`, `E_NOINTERFACE` or `E_NOTIMPL` at classified capability
sites can be candidates. Invalid descriptors/settings, busy/recovery conditions,
device loss, resource exhaustion, diagnostic setup/InfoQueue failures and unsafe
cleanup remain terminal. A nonempty `CGPUI_DX12_ADAPTER` or enabled
`CGPUI_DX12_DEBUG`/`CGPUI_DX12_GPU_VALIDATION` forbids fallback, preserving the
requested adapter and diagnostics. Malformed diagnostic settings are errors.

Checked rollback requires the same managed HWND instance on its owner thread,
no submitted frame/recovery, and no committed or ambiguous composition graph.
It retires the uncommitted native owners and explicitly checks alpha-token
release before allowing Vulkan to use the original descriptor. Failed surface
acquisition does not infer success from best-effort destructor cleanup. Raw or
foreign-owner HWNDs cannot provide this proof and may fail without fallback.
A successful opaque initialization on older Windows does not enable a later
backend switch when an ordinary-alpha request is unsupported.

Automatic successes report `RendererBackendSelection::automatic`, the actual
backend and live adapter/generation metadata. A Vulkan fallback owns the original
DX12 failure in `fallback_reason`; if Vulkan also fails, the returned error
retains both causes and the Vulkan error code. Runtime operations stay bound to
the selected backend, including pending work and device recovery. Linux and
macOS factory/default behavior is unchanged.

Named factories in `cgpui/renderer/renderer_backend.hpp` bypass that selection:
`create_vulkan_renderer(descriptor)` and, on Windows,
`create_dx12_renderer(descriptor)`. Application code can supply either through
its existing renderer factory; `RenderSurfaceDescriptor` has no new backend or
native/composition mode field.

Windows applications using `create_renderer` link `cgpui.renderer-windows`,
which owns both backend dependencies. Backend-only consumers may link the
named backend package. SDK link closure includes the dispatcher and both
backends. This link closure alone does not certify a DX12 release.
On Windows, neither the dispatcher nor the installed SDK imports `vulkan-1.dll`
at process startup. Explicit or successful automatic DX12 selection does not
load Vulkan, even though both backend libraries remain in the SDK link closure.
Only Vulkan initialization loads the DLL, through a private `LoadLibraryExW`
owner using
`LOAD_LIBRARY_SEARCH_DEFAULT_DIRS`: the application directory, System32 and
explicit user directories registered with `AddDllDirectory`. It neither searches
the current directory/ambient `PATH` nor changes the process DLL-search policy.

The loader resolves all **88 required exports** into a typed candidate table
before publishing any function pointer. Existing optional version and
device-extension queries retain their previous checks. Missing or damaged DLLs
return `renderer_initialization_failed` with `vulkan-1.dll` and the Win32 error
code; missing required exports return the same typed error naming the export.
A failed explicit Vulkan initialization does not select DX12. If Vulkan fails
after checked automatic fallback, selection stops with both initialization
causes; it does not cycle back to DX12. Failed binding attempts release
their module reference and can be retried; successful binding is serialized and
keeps its module reference for process lifetime, including late resource teardown.

Building the repository on Windows still needs the Vulkan SDK headers.
Consumers of a newly built installed Windows SDK need neither the Vulkan SDK nor
its import library or delay-load flags/hooks; they need a Vulkan runtime only
when explicitly selecting Vulkan or reaching the checked automatic fallback.
Older static-import SDKs keep their manifest-declared dependencies. Linux retains
its original direct Vulkan linkage. This is startup dependency isolation;
local automatic activation is not a new SDK release.

Run the native acceptance suite from PowerShell:

```powershell
$env:MCPP_JOBS = '1'
scripts/dev/test.ps1 -p tests/renderer-dx12
# Deterministic software rasterizer and strict native validation:
$env:CGPUI_DX12_ADAPTER = 'warp'
$env:CGPUI_DX12_DEBUG = '1'
scripts/dev/test.ps1 -Renderer dx12 -p tests/renderer-dx12
```

`CGPUI_DX12_ADAPTER` controls initial DX12 selection:

| Value | Behavior |
| --- | --- |
| Unset or empty | First suitable hardware adapter in `EnumAdapters1` order |
| Unsigned decimal index | Exact `EnumAdapters1` index; no alternative adapter |
| `high_performance` | Hardware-only `IDXGIFactory6::EnumAdapterByGpuPreference` high-performance order |
| `minimum_power` | Hardware-only native minimum-power preference order |
| `warp` | Explicit WARP software adapter |
| Anything else, including signs/whitespace or overflow | `invalid_argument` |

Preferred modes skip only unavailable device capabilities, not resource
exhaustion, device loss, busy recovery or other terminal failures. Missing
Factory6 or failed preferred enumeration is an error, not permission to use
legacy enumeration order. Every nonempty selector forbids automatic Vulkan
fallback. After creation, recovery stays on the recorded LUID regardless of
environment edits. These are diagnostic controls, not application window modes.

The native preference regression intentionally overrides an enclosing numeric or
WARP lane and independently queries native preferred order. Available hardware
must match its LUID, captured pixels and pinned recovery checks. If Factory6 is
absent or enumeration exhausts without suitable hardware, it must instead prove
that renderer initialization failed without Vulkan fallback, with an explicit
unavailable result in the log. Arbitrary oracle errors fail the test. This
branch is not hardware-rendering evidence or a silent skip.

Vulkan initialization separately reads `CGPUI_VULKAN_DEVICE` once. Unset/empty
keeps automatic suitability/alpha preference; an unsigned decimal uint32 index
requests exactly that initial `vkEnumeratePhysicalDevices` entry. Signs,
whitespace, trailing characters and overflow return `invalid_argument`.
An out-of-range or unsuitable selected device fails without trying another
adapter. Both ordinary and private Popup paths still require their full native
capabilities. Recovery pins the original physical-device identity within the
same instance, not a possibly reordered index, and never rereads the environment.
This setting does not select the Vulkan backend or alter successful automatic
DX12 selection; use `CGPUI_RENDERER=vulkan` to request Vulkan explicitly. It is
a private diagnostic override, not a public window-mode option.

`CGPUI_DX12_GPU_VALIDATION=1` requests GPU-based validation. Debug
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
Unset/empty/`auto` retains the common tests' existing named-Vulkan default.
This deliberately differs from the production Windows dispatcher's DX12-first
automatic policy: common pixel contracts must not silently substitute a backend.

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

## Scope and remaining acceptance

Dynamic ordinary-window upgrades, managed cross-thread recovery, native GPU
preferences, explicit Vulkan device selection and Vulkan recovery are implemented
and covered by the bounded attempt-7 Windows/WSL and ASan results above.
Performance sampling is complete within the stated scope, not a parity claim.
Client-title-bar ordinary alpha with an application HMENU, ordinary alpha on
hidden/undecorated chrome or older Windows remain unsupported. The
[ordinary transparency probe](dx12-ordinary-transparency-probe.md)
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

The shared rectangle/text/image/shadow/backdrop algorithms and DXIL are
implemented, and local automatic selection follows the bounded initialization
contract above. This does not extend the supported native-window topologies or
turn Vulkan results into DX12 evidence. The recorded Windows and requested-lane
checks are local bounded validation, not general native desktop/driver coverage,
hosted CI execution or release verification.
Steady-state reuse/pooling is not a performance-parity claim; deeper asynchronous
rendering remains optimization work. No release was published by this milestone.

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
