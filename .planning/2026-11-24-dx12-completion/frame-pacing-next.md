# Next slice: real DXGI frame admission, separate from GPU retirement

Date: 2026-11-24. Status: **read-only design, not implemented or executed**.
Written with explicit permission during the common-parity source freeze. No
production/test/manifest changes, engineering builds, or test runs accompanied
this note. Earlier planning snapshots are not evidence of current test results.

## 1. Current source facts

- `src/renderer/dx12/dx12_swapchain.cpp:7-8,37-52`: ordinary opaque HWND chains
  use `FRAME_LATENCY_WAITABLE_OBJECT` and `SetMaximumFrameLatency(1)`; composition
  chains use flags **0**, no maximum-latency call. No DX12 source currently calls
  `GetFrameLatencyWaitableObject` or waits on that object's handle.
- `dx12_renderer.cpp:24-29` checks recovery before constructing the CPU frame.
  `src/ui/render_view.cpp` begins the frame before `View::paint` and submission.
  This is the admission seam; adding a wait only immediately before Present is
  too late to satisfy the documented before-rendering ordering.
- `dx12_presentation.cpp:15-16` records `dx12_wait_idle` time as `pacing_wait`.
  That is GPU queue/fence synchronization, **not DXGI presentation admission**.
  Present is currently `Present(1, 0)`, followed by another fence wait before
  readback/texture commit. Do not delete either safety dependency as a pacing fix.
- `dx12_synchronization_native.cpp:16` currently uses an infinite fence wait;
  `dx12_retirement.cpp` polls until fence completion or genuine removal. These
  existing synchronization/teardown contracts are separate work. Adding a finite
  presentation wait does not make all renderer calls bounded or all UI stalls fixed.
- `dx12_native_retirement.cpp:10-42` is the shared native-release owner, including
  recovery/cohort retirement and partial initialization cleanup. It releases the
  composition host and swapchain before queue/device teardown.
- `src/ui/runtime_frame_retry.cpp:11` immediately requests another normal render
  for `frame_retry_required`; there is no backend-readiness delay in this result.
  `win32_window_proc_lifecycle.cpp:88-96` validates WM_PAINT before invoking the
  runtime; `win32_application.cpp:45-53` owns the real GetMessage/DispatchMessage loop.
- Opaque rendering on a worker while the HWND owner pumps messages is supported:
  `dx12_window_alpha.cpp:41-58` and
  `tests/renderer-dx12/dx12_ordinary_opaque_thread_test.cpp`. Only the managed
  ordinary-alpha lease requires the HWND owner thread. Do not add a blanket
  owner-thread restriction to opaque rendering while adding pacing.

## 2. Microsoft contracts checked directly

The following are current primary documentation, fetched read-only for this note.

1. [GetFrameLatencyWaitableObject][get]: create with the waitable flag, retrieve
   the handle, wait before **every frame including the first**. The documented
   return is NULL when the flag is absent. Crucially, **the application calls
   CloseHandle when finished**. This is not a borrowed handle owned solely by
   the COM swapchain. Do not leak it, and do not CloseHandle it twice.
2. [SetMaximumFrameLatency][set] applies only to chains created with that flag;
   otherwise it returns `DXGI_ERROR_INVALID_CALL`. This is per-swapchain, not
   `IDXGIDevice1::SetMaximumFrameLatency`. [The lower-latency guide][guide] says
   the default is 1 and explains the throughput/latency trade-off of 2.
3. [The lower-latency guide][guide] explicitly says the waitable flag **cannot be
   added or removed by ResizeBuffers**. Keep the original flags across resize.
   This specific rule qualifies the broad flags wording in [ResizeBuffers][resize].
4. [WaitForSingleObjectEx][wait] says closing a handle while a wait is pending is
   undefined behavior. A zero timeout returns immediately; `WAIT_TIMEOUT` is not
   device removal. Non-alertable waits cannot validly use `WAIT_IO_COMPLETION`
   as proof of frame readiness. GetLastError is meaningful for `WAIT_FAILED`, not
   arbitrary successful/timeout results.
5. [WaitForSingleObjectEx][wait] warns against indefinite waits on threads that
   own windows. [MsgWaitForMultipleObjectsEx][msgwait] can observe both handles
   and input; queue wake (`WAIT_OBJECT_0 + count`) is not handle readiness.
   `MWMO_INPUTAVAILABLE` changes old/unread input behavior. Merely swapping the
   function name does not establish a safe runtime/reentrancy policy.
6. [Present][present] can interact with the window's message-pump thread and
   return removal/reset or `DXGI_STATUS_OCCLUDED`. [DXGI programming guidance][dxgi]
   recommends standby/testing presentation for occluded windows. A minimized or
   occluded window must not turn a missing pacing signal into a tight redraw loop
   or an invented successful frame.
7. [CreateSwapChainForComposition][composition] requires FLIP_SEQUENTIAL and
   STRETCH; for D3D12 its device argument is a direct queue. These docs do **not**
   establish that every supported composition/OS/adapter configuration in this
   repository accepts the extra waitable flag. Nor do they establish a universal
   prohibition. Treat composition pacing as a separately measured capability,
   not as a reason to disable Popup or expose a public window mode.

The docs do not promise an event reset mode, a new unique numeric HANDLE on each
query, exact refresh intervals, a particular signaling schedule while occluded,
or that an admission signal proves any D3D12 fence ticket completed. Do not infer
these as portable contracts from one driver.

## 3. Minimal module ownership

Proposed **private** modules, not additions to renderer aggregate headers:

| Owner | Responsibility |
| --- | --- |
| `dx12_frame_pacing_internal.hpp/.cpp` | Deep module owning one swapchain's handle, observed admission grant, reset/close and state machine. |
| `dx12_frame_pacing_native_internal.hpp/.cpp` | Narrow native adapter for latency configuration, handle retrieval, wait result plus captured error, and close. |
| `dx12_frame_pacing_frame_internal.hpp/.cpp` | Small state/frame integration: recovery-before-admission, begin/present ordering and diagnostic transfer. Add only if it avoids growing the existing entry/submit files. |
| `Dx12State` | Own a `unique_ptr` to the pacing module; no duplicate loose HANDLE/booleans or worker implementation here. |

Sketch of the module interface, not final names or implementation:
`configure(chain, enabled) -> Result<void>`,
`try_admit() -> Result<ready | pending>`,
`presentation_attempted()`, `reset() noexcept`.
Disabled is an explicit internal route for the current non-waitable composition
chain, not a successful native wait. The owner does not retain an extra COM
device/queue/swapchain reference that could pin recovery's old singleton.

The native adapter and scripted test adapter are the two real implementations
at the private seam. Production always selects the real adapter. An injected
adapter's lifetime must outlive the state; no public environment variable or
factory branch may turn genuine acceptance tests into fake waits.

Keep headers declaration-focused (<=200 lines) and implementation owners <=280
lines. Add structure checks for ownership, lifecycle hooks and no public native
handle leakage. Do not bury the new policy in `dx12_renderer.cpp`,
`dx12_presentation.cpp`, broad runtime state or Win32 application files.

## 4. Lifecycle and ordering

### Creation and frame admission

- On successful waitable swapchain creation: set maximum latency, query the
  handle **once**, reject NULL explicitly, and adopt it immediately. Cleanup
  must also work if later RTV/target/debug/composition initialization fails.
  GetFrameLatencyWaitableObject has no documented GetLastError contract; do not
  report an unrelated stale Win32 error for its NULL result.
- `begin_frame`: validate existing alpha/thread constraints, outstanding-frame
  state and recovery first; then acquire admission; only then publish a frame
  and set `frame_active`. A pending result must not invoke paint/upload/submit,
  change generation, overwrite the last successful diagnostic, or report success.
- There is one consumer per chain, serialized by the existing operation guard.
  No nested message pump while that recursive mutex/operation scope is held.
  Do not dispatch arbitrary WM messages from a backend wait implementation.
- A ready wait is an **observed admission grant**, not a fence ticket. Track it
  until the corresponding Present attempt. A CPU frame abandoned before GPU
  submission/Present must not consume the sole readiness observation and then
  blindly wait for an event that only a future Present could cause. This case
  requires a real abandoned-first-frame regression; do not assume reset mode.
  Retaining an unconsumed grant is not manufacturing a new signal.
- Invalidate the grant on every actual Present attempt; do not recycle it based
  on a success-shaped HRESULT or a guessed occlusion policy. Failure/removal
  follows the existing recovery path. Grant/error semantics on pre-Present
  failures need focused tests alongside independent fence-retirement tests.

### Resize, recreation, recovery and destruction

- Same COM swapchain `ResizeBuffers`: retain the original flags and owned
  handle; do not obtain/close a fresh handle each frame or resize. Preserve an
  unconsumed admission observation associated with that same chain unless real
  native evidence requires a different documented rule. Verify resize before
  first Present and after an abandoned frame, not only steady-state resize.
- New swapchain/recovery: reset/close the old handle exactly once before the old
  swapchain is released, clear its grant/epoch, and acquire the replacement
  chain's handle. Never carry a grant across chain recreation, even if the OS
  reuses the same numeric HANDLE. Partial rebuild failures must also reset it.
- Place reset in `dx12_release_native_objects`, before `swapchain.Reset()` and
  after the caller's existing completion/removal/never-submitted proof. This
  covers cohort peer retirement too. Pacing reset does not supply that proof.
- No background waiter in the first ownership slice. A synchronous zero-time
  probe ends before reset under the state lock, so there is no pending-wait
  close race. If a later event-driven waiter is introduced, unregister/cancel,
  drain callbacks/waits, reject stale chain epochs and only then CloseHandle.
- Renderer destruction with an outstanding CPU frame follows existing shared
  state lifetime. No raw observer, HWND or runtime callback may outlive its
  owner. Never block the HWND owner waiting for a worker that may be inside
  Present/SendMessage to that owner. Preserve the opaque worker-render contract.
- Existing alpha-intent changes retain the same composition topology/chain;
  they must neither invent an HWND-chain pacing handle nor close/reopen one.

## 5. Scheduling decision needed before enabling the gate

**Reject the tempting one-line implementation:** `WaitForSingleObject(INFINITE)`
inside begin/Present. It can block the UI and make shutdown/resize unserviceable.
Also reject `wait(0) -> frame_retry_required -> immediate redraw` as the complete
design: current runtime retry would continually re-invalidate an unready window.
Queue wakes from MsgWait are not a reason to render without a signaled grant.

Recommended smallest correctness slice: **nonblocking native admission plus a
coalesced delayed runtime pending path**. `WaitForSingleObjectEx(handle, 0, FALSE)`
is a real native readiness observation and introduces no new UI-thread wait.
The existing synchronous Renderer contract needs an explicit neutral pending
outcome to carry that result safely; a proposed `frame_not_ready` ErrorCode is
one small option, subject to interface review before implementation. Keep it
distinct from `frame_retry_required`, which already represents a rejected frame
with immediate recovery retry and has root/child coverage. Never parse messages
or backend names to distinguish pacing, and do not expose HWND/DXGI types or
developer-selected window modes in that neutral result.

The runtime companion belongs in a focused private frame-readiness module:

- At most one pending wake/timer per runtime window. Use existing
  `schedule_timer`/`cancel_timer` and ordinary `Window(...).request_render()`;
  do not synthesize redraw events or call `try_draw_frame` recursively.
- A bounded nonzero retry delay yields the message loop. Delay/backoff values
  are policy to measure, not an assumed 16.7 ms refresh rate. Collapse duplicate
  WM_PAINT/user invalidations while pending; an early real paint may probe once
  without creating a second pending timer. Cancel on a successful admitted
  frame, close, runtime shutdown or renderer replacement; stale IDs/epochs must
  not draw through destroyed objects. Preserve fairness between root and child.
- `WAIT_TIMEOUT` means ordinary pending/backpressure. Do not mark the device
  failed or recreate it, and do not terminate a hidden window just because a
  wall-clock threshold expires. Back off hidden/occluded pending requests while
  allowing visibility/input/close messages. Persistent no-progress reporting is
  diagnostic, not authority to retire submitted resources.
- `WAIT_FAILED`: capture GetLastError immediately; map zero to a non-success
  fallback error. Check genuine device-removal state through the existing owner
  and recover only with actual evidence. Unexpected abandoned/alertable statuses
  fail explicitly; they cannot manufacture readiness. No infinite internal loop.

This timer route is a **functional admission implementation**, not proof of
lowest input-to-display latency: timer granularity/backoff may add latency and
must be measured. If this is unacceptable, use a second, explicitly designed
neutral readiness-notification seam with native wait registration and a safe
coalesced UI wake. Do not quietly add a threadpool callback that consumes the
event and lets begin wait for the same signal again. Callback lifetime,
cancellation, duplicate signal consumption and handle-close races would become
additional mandatory tests. No event-loop redesign is authorized by this note.

The public pending-result decision and its compatibility implications must be
resolved first. Direct Renderer clients currently commonly expect immediate
begin success; update that contract and their legitimate bounded pending handling
together, not by turning all errors into retries. Existing recovery tests must
continue to distinguish genuine recovery retries from presentation backpressure.

## 6. Composition and diagnostics scope

Start with the ordinary opaque route that already requests the flag. Keep Popup
and initially-transparent ordinary windows on their existing composition path
until real capability/lifecycle checks are available. This is a limitation of
the first pacing slice, **not full all-window pacing completion**. Investigate
composition flag + SetMaximumFrameLatency + nonnull handle on actual adapters;
do not present a failed experiment as universal API prohibition, silently change
window topology, restore Vulkan-to-Overlay preemption, or disable Popup.

Keep `pacing_wait_nanoseconds` truthful. Admission occurs before the current
Dx12FrameDiagnostics constructor, so measure in the new owner and transfer only
the admitted frame's CPU wait/probe interval. Do not reuse stale durations,
double-count them, or label delayed event-loop residence as blocked CPU time.
The existing pre-submit fence interval must remain identified as synchronization;
if the present aggregate field intentionally includes both, document the split
and add an internal oracle rather than claiming the whole number is DXGI wait.
The timing ledger must still sum exactly and publish only successful frames.
No timing threshold alone proves a real waitable-object call.

## 7. Verification gates, fake versus native

**Private scripted failure tests:** configure failure; NULL acquisition; exactly
one close on success/partial failure/rebuild; no close on disabled composition;
ready/pending/WAIT_FAILED with stale GetLastError; unexpected status; cached
unconsumed grant; first frame, abandoned frame, pre-Present error and Present
attempt transitions. Use fake handles only with the fake close/wait adapter.
Check that latency readiness does not change `submitted`, fence ticket/completed
value, upload retirement, command allocator lifetime, or device generation.
These tests prove the state machine, not driver pacing or hardware behavior.

**Real native tests:** on Intel/NVIDIA/WARP, real flagged HWND chain, actual
nonnull handle and GetMaximumFrameLatency=1; first/repeated frame ordering via a
strictly forwarding private native adapter (records real calls/results, never
substitutes them); real frame capture/pixels; resize; abandoned first/cold frame;
repeated create/destroy with close accounting; RemoveDevice/recovery and cohort
peer recreation; opaque worker rendering with owner message pumping. No exact
numeric handle-inequality assertion, FPS threshold, or sleep duration proves it.

**Native runtime tests:** pending admission returns control so input, timer,
resize and close can be delivered; root and ordinary child remain fair; no
after-frame callback or capture is published for pending frames; no synthetic
success; no hot loop under repeated timeout/occlusion; restoration resumes
genuine rendering. A controlled fake-pending test proves scheduler logic; an
actual waitable DX12 + real event loop test proves the integration. Do not relabel
the former as native acceptance. Process watchdogs may bound tests, never justify
releasing in-use resources in production.

**Regression/structure:** all current fence terminal/retirement tests, runtime
recovery, ordinary-alpha/Popup/worker-thread behavior, common Vulkan/DX12 pixels,
debug/GPU validation, ASan, neutral headers and non-Windows builds. Add structure
guards requiring the private pacing leaf, centralized acquisition/close, no
INFINITE latency wait, no pacing handle in public headers, and no use of pacing
readiness in native GPU retirement decisions.

## 8. Explicit remaining uncertainties

Native signaling through occlusion/minimize, same-chain resize and abandoned
admission; compatibility of composition waitable flags on the supported matrix;
timer-based wake granularity/latency; and the smallest neutral pending interface
are **not measured here**. They are prerequisites/acceptance questions, not facts
to hide behind a successful ordinary present. Existing infinite fence waiting
also remains a separate responsiveness risk, not solved or weakened by this plan.

## 9. Concrete minimal scheduling recommendation after API/source review

This section resolves the alternatives in section 5 into an implementable first
slice. It remains design only; no public interface has been changed or approved
by writing it.

### Decision: existing window-bound one-shot timers, not a native wait worker

Use the **existing WindowRuntime timer/dispatcher path** to schedule one future
normal redraw per unready window. Add only one backend-neutral pending result
to the existing begin-frame contract. Do not introduce a Renderer callback API,
export a wait handle, install a per-swapchain threadpool waiter, or make DX12
send WM_PAINT/InvalidateRect/private framework messages to its HWND.

Source evidence makes this smaller than a new dispatcher:

- `runtime_context_scheduling.cpp:24-27` already routes a context timer to
  `schedule_window_timer(window_runtime_id, ...)`.
- `runtime_window_timers.cpp` binds the callback to the originating window and
  marks `RuntimeTimer.window_runtime_id`. `runtime_window_callback_binding.cpp`
  resolves the current child record and suppresses callbacks to inactive or
  missing children; it does not silently redirect a stale child to root.
- `runtime_window_records.cpp` and `runtime_window_shutdown.cpp` already cancel
  a child's bound timers before its renderer/window is retired.
- `runtime_timer_integration.cpp` multiplexes the earliest timer onto
  `PlatformApplication::request_wakeup_after`; `runtime_timers.cpp` removes a
  one-shot timer before invoking its callback. The normal Win32 timer callback
  posts a runtime wake, and only the owning event-loop thread touches runtime
  state/requests redraw. These are the required capabilities already present.

| Alternative | Why not the first slice |
| --- | --- |
| `context.defer` / foreground task | Runs on the next drain, not a future deadline. Repeated pending results can still self-wake continuously. |
| Background task sleeping/waiting on the native handle | Occupies the shared task pool, adds cancellation/join races, and needs an otherwise absent safe handle handoff. An indefinitely blocked worker can block `RuntimeTaskPool::shutdown`. |
| AsyncIoHook | Provides safe completion notification, but not a waitable-object registration or cancellation/drain ownership. Passing this UI-owned hook into a renderer would create another cross-module contract, not reuse an existing renderer seam. |
| Call application.request_wakeup_after directly | Its timer is application-wide; bypassing the runtime's earliest-deadline multiplexer can replace a user's earlier timer. It also does not identify which window to redraw. |
| New Error payload containing absolute deadline | Unnecessary for this slice; introduces clock-domain/serialization policy into core errors. The runtime already owns a monotonic scheduling clock. |
| New renderer readiness callback/registration | Potential later latency optimization, but adds callback ownership, cancellation, native wait drain and consumed-signal races. Do not add it before the timer design is measured. |

### The smallest necessary neutral seam

Existing public `begin_frame()` returns only a frame or `Error{code,message}`.
It cannot distinguish temporary presentation backpressure from an immediate
post-recovery retry. Reusing `frame_retry_required` would either hot-loop or
change the existing recovery scheduling contract for every backend. A null
successful frame would violate the existing frame contract. Message-string
parsing or `backend_info().backend == dx12` scheduling would be hidden coupling.

Therefore propose, for explicit interface approval, **one appended ErrorCode**:

```cpp
// begin_frame acquired no frame: presentation admission is temporarily pending.
// Nothing was submitted and the renderer remains usable. Schedule a later
// attempt; do not report a successful frame or classify this as device failure.
frame_not_ready,
```

`Renderer::begin_frame()` keeps its exact signature
`Result<std::unique_ptr<RenderFrame>> begin_frame()`; `Error` gains no fields.
Append the enumerator to preserve current integer values. Document this result
as begin-only in `renderer_frame.hpp`; Present/resize do not use it in this
slice. The code name may be revised during interface approval, but its distinct
semantics are necessary. No application chooses a window/presentation mode, and
Vulkan/custom backends need no new override or timer setup.

Only a genuine native pending wait produces this result in DX12. Device removal
still uses the existing real recovery path and `frame_retry_required`. Timer
expiry is **permission to attempt**, never permission to render: each attempt
must again cross the native admission module (or use its genuinely observed,
unconsumed same-chain grant).

### One focused runtime owner and exact state transitions

Use a small private `RuntimeFrameReadiness` owner in
`runtime_frame_readiness_internal.hpp/.cpp`. `RuntimeScheduleState` gains one
owned object/pointer, not scattered per-backend fields. The owner keeps entries
keyed by `WindowRuntimeId`: current TimerId, a monotonically increasing token,
run epoch, and the start time of continuous pending. No Renderer, RenderFrame,
PlatformWindow, HWND, DXGI handle or callback-owner raw pointer is stored in a
timer capture. Private callbacks capture only `(window_id, token, run_epoch)`
and resolve the owner's current entry through the supplied live runtime context.

Suggested private operations are `pending(runtime, id)`, `clear(runtime, id)`,
`clear_all(runtime)` and the token-checked timer delivery; no public method is
needed. A nested private class/declaration fragment can use existing
`schedule_window_timer` without promoting that method into the public API.

1. **Actual begin reports pending.** `render_view` returns before paint/upload;
   existing root abort scheduling clears its frame/redraw flags, and the child
   path already clears its redraw-scheduled flag before drawing. The root/child
   redraw handlers pass this typed outcome to `pending`, suppress fatal error
   and successful-frame callbacks, and return to the event loop. They do not
   call `Window::request_render` immediately.
2. **Arm once.** If the entry already has a timer, leave its deadline/token
   unchanged. Otherwise allocate a new token and use
   `schedule_window_timer(id, delay, false, callback)`. Explicit prototype policy:
   2 ms while continuously pending for less than 250 ms; 64 ms thereafter.
   These are private scheduler constants, not a refresh-rate claim or a public
   tuning API. Reset the pending age only on successful presentation or clearing
   that lifetime, not on every pending result. A positive delay is mandatory.
3. **Deliver once.** Runtime removes the one-shot timer before callback. Check
   run epoch/token and an active current native record (including root), then
   mark the entry's TimerId empty **before** calling `context.request_render()`.
   This uses the existing deferred child/root scheduling. Do not render from
   the timer callback. The next actual WM_PAINT performs the next attempt; if
   still pending it arms a new positive-delay timer.
4. **An earlier OS/user redraw is legal.** Let it try native admission once. If
   still pending, retain the same outstanding timer; if successful, cancel the
   timer and erase the entry. Do not add an early WM_PAINT rejection layer in
   this slice: skipping normal frame bookkeeping can leave redraw_scheduled set
   and swallow the timer's future request. The design bounds *self-generated*
   retry wakeups; it does not pretend to suppress externally requested paints.
5. **Successful present.** Clear/cancel the readiness entry in the successful
   frame-result owner **before** invoking user after-frame callbacks. A copied
   stale timer callback fails its token check. Queued ordinary WM_PAINT cannot
   necessarily be retracted, but is at most an extra legitimate frame, not a
   retained dangling callback or a self-rearming timer.
6. **Real recovery retry or fatal error.** Clear any pending timer/age first.
   `frame_retry_required` follows the existing immediate recovery retry path;
   other errors remain fatal. Do not count a pacing pending as a device-generation
   transition or as the recovery retry oracle used by the current native tests.

This policy introduces no per-frame sleeping worker and no new busy retry loop.
The first 250 ms permits inexpensive readiness probes; sustained hidden/no-signal
windows back off to at most roughly one self-request per 64 ms. That does not
promise zero hidden-window cost or latency-optimal pacing. Timer granularity
and the new pending path's repeated View::render CPU work must be profiled:
View::paint is skipped on pending, but the runtime currently builds its View
tree before entering `render_view`. Do not call the entire CPU render path zero
work. Ordinary visibility/input-triggered paints can attempt earlier than a
backoff timer, so the delay is not an artificial embargo on user interaction.

### Complete cancellation and native-lifetime interaction

- **Child close/deactivation:** retain existing window-bound timer cancellation,
  and erase the dedicated readiness entry as part of the same cleanup. This
  cannot cancel unrelated user timers beyond the existing close semantics.
- **Root cleanup / repeat run:** `reset_root_window_resources` currently does
  not call `cancel_window_timers(root)`. Explicitly clear the root's *readiness*
  TimerId before detaching its renderer; do not erase all root user timers as a
  side effect. `clear_all` runs at session teardown/start and increments a run
  epoch. This matters because the same WindowRuntime can run again with reused
  window IDs while a copied old timer closure may still exist.
- **Shutdown:** clear entries/timers once should_quit/shutting_down is set and
  before retiring renderers. Cancel only tracked pacing TimerIds. Existing timer
  multiplexer handles remaining user deadlines; do not call application-wide
  `cancel_wakeup_after()` independently. A late native wake has no matching
  pacing timer and does not recreate one.
- **Same-window resize:** retain the timer; it identifies a logical runtime
  window, not a swapchain. Do not cancel its sole future redraw unless the resize
  path guarantees a replacement request (current resize handlers do not). The
  backend independently preserves or replaces its handle/grant according to
  section 4. If a resized/recovered chain later returns pending, the one timer
  remains sufficient. A successful frame resets age; a true recovery retry
  clears it and schedules the immediate recovery route.
- **Renderer replacement/record retirement:** clear the entry before detaching
  the renderer, just as for root/child cleanup. Token invalidation, not comparing
  recycled raw pointers or HWND numbers, rejects stale deliveries.
- **CPU frame abandonment:** no runtime timer owns the frame. A native admission
  observation remains owned by the same swapchain until an actual Present
  attempt; dropping an admitted-but-unpresented CPU frame cannot consume a future
  timer callback or grant permission to retire GPU work. A direct client chooses
  its next begin attempt; no hidden WM message is sent on abandon.
- **RemoveDevice / cohort reconstruction:** the backend closes/replaces the old
  pacing handle without outstanding asynchronous waits; the runtime timer owns
  no old-device reference and cannot block replacement. Actual recovery runs
  before admission on the next attempt. Timer delivery cannot certify recovery.
- **Opaque worker/direct client:** no runtime exists to schedule its retry.
  The direct caller receives the same neutral pending result and uses its own
  bounded retry/deadline/cancellation mechanism. It must continue pumping HWND
  messages on the owner thread while the renderer worker runs, as existing tests
  require. DX12 never tries to invoke UI/runtime APIs, uses the opaque alpha
  handshake as a covert scheduler, or accesses PlatformWindow from the worker.
  The existing owner-thread-only managed alpha contract remains unchanged.

### Required delayed-wakeup failure safeguard

Do not assume the existing timer transport can always delay: in
`src/platform/win32/win32_timer_wakeup.cpp`, a failed `CreateThreadpoolTimer`
currently makes `request(delay)` call `post_wakeup()` immediately. A future-due
runtime timer then re-arms, receives another immediate wake and can hot-loop.
`PlatformApplication::request_wakeup_after` returns void, and its base
implementation in `src/platform/empty.cpp` is a no-op, not a capability guarantee.

For the concrete native Windows slice, make a usable Win32TimerWakeup an explicit
private platform-initialization condition and return existing
`platform_initialization_failed` if it cannot be created. This is a small
fail-closed change to that private owner/factory, not a new public Renderer or
PlatformApplication callback/return type. Capture the timer-creation Win32 error
at failure. Test its failure branch instead of allowing immediate fallback to
masquerade as a delayed scheduler. Other platform implementations/custom test
platforms must honor the existing delayed-wakeup contract or deterministically
advance timers; this slice does not claim their timer support from the base stub.

### Small, separately provable implementation order

1. Approve/document the one neutral begin-only pending result; add private
   admission owner and scripted error/lifetime tests without fake native success.
2. Add the window-bound readiness owner and tests using TestPlatformApplication
   with its controlled clock: no immediate redraw, no wake before the deadline,
   exactly one timer per window, early successful paint cancellation, token/run
   expiry, child/root close, same-window resize and independent user timers.
3. Cover Win32 delayed-timer allocation failure and native wake transport. This
   is separate from DXGI readiness and must not require a synthetic native device.
4. Enable genuine DXGI admission; run direct/native-runtime acceptance, including
   real first frame and abandoned first frame, typed recovery unaffected, no
   hidden HWND scheduling, actual input/close delivery while pending, and worker
   rendering. Measure 2 ms/64 ms policy and actual timer granularity before calling
   the result performance-complete. Change private constants only with evidence;
   a later event-driven neutral registration is justified only if this measured
   simple solution cannot meet the actual latency requirement.

This is a selected, bounded implementation path rather than a menu of equally
recommended designs. The only proposed public delta is a neutral result code;
all scheduling work reuses existing runtime timers and normal Window redraw.

[get]: https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_3/nf-dxgi1_3-idxgiswapchain2-getframelatencywaitableobject
[set]: https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_3/nf-dxgi1_3-idxgiswapchain2-setmaximumframelatency
[guide]: https://learn.microsoft.com/en-us/windows/uwp/gaming/reduce-latency-with-dxgi-1-3-swap-chains
[resize]: https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-resizebuffers
[wait]: https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobjectex
[msgwait]: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-msgwaitformultipleobjectsex
[present]: https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-present
[dxgi]: https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/d3d10-graphics-programming-guide-dxgi
[composition]: https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/nf-dxgi1_2-idxgifactory2-createswapchainforcomposition
