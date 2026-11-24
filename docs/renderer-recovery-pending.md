# Unfinished renderer recovery

`renderer_recovery_pending` is a distinct temporary result, not a successful
render, completed reconstruction, or presentation-admission grant. It adds no
backend or window-mode option. Recovery never switches the selected backend;
[automatic initialization and explicit selection](dx12-backend.md#selection-and-testing)
remain separate from this protocol.

- `begin_frame`: no frame is acquired. Schedule a delayed attempt.
- `present`: the current frame terminates without successful presentation. Discard
  it and later start a new frame; do not resubmit the old frame.
- `resize`: the requested extent/scale is not yet applied. Preserve only the latest
  desired request, and successfully reapply it before drawing at that extent.

`frame_not_ready` remains begin-only presentation backpressure.
`frame_retry_required` still means recovery completed and a new frame is required.
Neither is an alias for unfinished device recovery. All other failures retain
their ordinary error semantics; pending does not justify indefinite native waits,
discarding busy resources, or silently selecting another adapter/backend.

WindowRuntime uses its existing coalesced, window-bound delayed redraw timer for
both admission and unfinished recovery, rather than an immediate retry loop.
Root and additional windows share a private latest-resize owner. Their window
geometry represents desired layout; while resize is pending no frame is acquired
against the previously applied native surface. Multiple resize/DPI events replace
the desired request. A resize callback cannot recursively apply an obsolete
extent: the latest revision is retried before drawing. Closing a window, runtime
shutdown and a new runtime session cancel both pending resize and delayed wake.
Fatal resize errors follow the same reporting/termination policy on root and
additional windows.

Direct Renderer clients own their scheduling, HWND lifetime and pending resize
storage. A delayed retry is not a promise that external references or an unpumped
owner will eventually cooperate. Backend recovery deadlines and terminal errors
still apply.

## Implementation status

This contract and its Runtime/DX12 implementation are implemented. The
[2026-11-25 status](dx12-backend.md#current-validation-status-2026-11-25) records
fresh attempt-7 Windows, hardware/WARP, local SDK, typed WSL and fixture results,
including bounded managed-owner recovery and Vulkan selector/reconstruction tests.
Current ASan passed; 18 performance series completed with consistent raw outputs,
not an FPS, GPU-time, speedup or performance-parity claim.
No hosted CI, release or arbitrary driver-reset acceptance is asserted, and not
every backend emits this optional pending result.

## DX12 owner-dispatched retirement

The DX12 implementation tracks a private CPU-only native lease
before `D3D12CreateDevice`, including partial construction. A loss wave seals new
attachments on that adapter while its known participants remain unresolved. A
participant acknowledges retirement only after releasing its actual native
references; an expired weak state does not mean its destructor has finished.
An independently observed healthy lease is unaffected rather than falsely
declared removed. These identities are internal coordination tokens, not public
device generations or a claim about external D3D12 owners.

The triggering operation retires its own native objects and returns pending
without waiting for another window thread. Same-owner idle peers retain the
nonblocking fast path. Foreign managed windows receive integer instance/work
tokens through their private message endpoint. Dispatch runs on the actual owner
thread, tries the state gate without waiting, checks operation depth and actual
device removal, and retires only the requested native lease. Busy work is not
acknowledged or forcibly released. The outer operation can post a coalesced wake
after releasing its gate. No callback address/state pointer is queued, no hidden
worker or synchronous cross-thread message is introduced, and replacement still
occurs through the renderer's legal public entry points.

Each wave has an absolute five-second cooperation deadline; retries do not
extend it. An unavailable managed endpoint, failed post or uncooperative owner
produces an explicit terminal error, not a successful frame. If real release
later removes the blockers, a direct client's later explicit call may recover;
the Runtime still treats terminal errors as fatal. Unmanaged HWND rendering is
not removed, but foreign automatic retirement requires a managed endpoint.
Neither a timeout nor token cancellation authorizes releasing active GPU work.

Queued work is tied to a managed HWND instance and its owner thread. Window
destruction cancels the endpoint; HWND reuse cannot consume old tokens. Registry
locks protect metadata only, and weak promotion, native callbacks and final
strong-reference destruction happen outside them. The enqueue-only transport is
serialized with coalescing so a failed native post cannot be mistaken for an
accepted wake by another sender.

DX12 recovery reopens only its recorded adapter LUID. Neither numeric adapter
selection nor `high_performance`, `minimum_power` or `warp` is reread during
reconstruction; editing the environment cannot repin an existing renderer.
All nonempty initial selectors prohibit automatic Vulkan fallback. They remain
diagnostic controls, separate from the owner-retirement protocol.

Bounded local tests do not certify unpumped windows, arbitrary driver reset/TDR
or another module's native owners. DRED remains separately bounded, non-coherent
diagnostic sampling, not retirement authority. No backend/window policy selector
or runtime backend fallback is added.

## Vulkan recovery is synchronous, not pending

The new Vulkan path uses raw native `VK_ERROR_DEVICE_LOST` at explicit
wait/acquire/submit/present/resize/readback boundaries, including loss discovered
during failed-operation retirement. Non-loss errors cannot authorize a rebuild.
It synchronously attempts one device reconstruction while retaining the instance,
surface, CPU glyph cache and accepted image-upload payloads. Automatic Vulkan
selection prefers the prior physical device if still suitable, but may choose
another suitable Vulkan device. Explicit `CGPUI_VULKAN_DEVICE=<index>` selection
instead pins the initially chosen `VkPhysicalDevice` identity within the same
instance. Recovery does not reread that environment variable or reapply an index
to a changed enumeration; a missing/unsuitable pinned device fails. DX12 remains
pinned to its original LUID. Neither backend switches graphics APIs.

Full native reconstruction advances generation. Recovery during presentation
returns `frame_retry_required`; the lost frame was not successfully presented. A failed
rebuild retires partial resources and keeps a terminal error for later calls,
not `renderer_recovery_pending` or repeated automatic reconstruction. Vulkan
does not join DX12's managed-owner cooperation waves.

Frames from the retired generation cannot apply uploads or invalidations to the
new device. Current-generation frames become single-use when present starts;
retry requires a new frame, not replay of moved upload buffers. Retained images
honor replacement, explicit invalidation, allocation identity and normal idle
eviction; restoration checks the replacement device's limits even when the new
frame draws no images. GPU upload prefixes restart from the retained CPU content.
The most recent successful CPU capture remains available through native
retirement and failed reconstruction, without publishing a failed candidate.

If loss is discovered by Vulkan resize, reconstruction uses the requested
extent/scale. Resize returns success only after complete native reconstruction
and a generation advance; non-loss errors and failed reconstruction remain
errors. It does not return `frame_retry_required` or unfinished owner cooperation.
This matches WindowRuntime's shared contract: successful resize applies the
requested state before drawing, rather than reinterpreting a frame retry as
successful resize or deferred work.

The native Vulkan regressions first drain real healthy native devices and then
inject a typed failure at the selected import. They check real native ownership,
generation and image/glyph pixels; the smaller-limit case is a separate controlled
capability simulation. They are not physical device removal, adapter migration
or TDR evidence. These tests passed in the attempt-7 Windows regression and
common-suite lanes; that does not extend the simulated-failure evidence scope.
