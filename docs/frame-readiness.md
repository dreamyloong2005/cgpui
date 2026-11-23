# Renderer frame readiness

`Renderer::begin_frame()` may return `ErrorCode::frame_not_ready` when it has
acquired no frame and submitted no work because presentation admission is still
pending. The renderer remains usable. This result is **begin-only**: `present()`
and `resize()` must not use it. No successful frame/capture is implied.
`render_view` converts a present-returned `frame_not_ready` to the fatal
`frame_acquisition_failed` code, with a contract-violation prefix and the original
error message preserved. Other present errors retain their code and message.

The WindowRuntime readiness implementation schedules a coalesced window-bound
one-shot timer and then requests an ordinary future redraw. Repeated pending
results do not immediately redraw or recreate a renderer. Early success cancels
the timer before after-frame callbacks. Recovery's existing
`frame_retry_required` remains a separate immediate retry; actual fatal errors
are not turned into presentation retries.

Direct Renderer clients own their event-loop scheduling and should arrange a
bounded/cancellable future attempt without busy-waiting. A timer firing is only
permission to try admission again, not evidence the backend is ready. Neither
readiness nor a successful presentation-latency signal proves GPU retirement.

DX12 opaque HWND swapchains now poll their frame-latency handle once, without
blocking, before constructing a frame (including the first frame). A native
timeout returns this pending result. An observed ready grant survives abandoned
frames and pre-Present validation failures; every actual Present attempt consumes
it. Same-chain resize preserves the handle, and recovery closes/reconfigures it.
Composition chains currently use no waitable flag and do not claim admission
pacing. Vulkan and Metal do not currently emit this result. There is no public
window mode, renderer callback/registration method, or native wait handle.

The initial runtime policy polls at a private short delay and backs off sustained
pending windows. These intervals are not refresh-rate or latency guarantees.
Controlled-clock runtime tests cover scheduling independently of the GPU. Native
DX12 root/child recovery tests also exercise actual pending results through the
Win32 event loop; their observers return pending unchanged and never wait/retry
on the runtime's behalf. These are functional tests, not refresh-rate guarantees.
Platform implementations must supply a functioning delayed-wakeup transport;
the base PlatformApplication no-op is not such a transport.
