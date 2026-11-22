# Runtime window and rendering lifecycle

The runtime owns lifecycle policy; applications do not select an operating-system
composition mode. This contract applies to ordinary child windows and independent
popups as well as the root window.

The X11 event loop drains XCB's already-buffered events before blocking on the
socket. A synchronous protocol reply may have consumed the socket bytes without
dispatching an earlier event; waiting for another socket notification would
otherwise strand the window's first redraw or input.

## Installed API and state encapsulation

`WindowRuntime` contains one owning opaque-state pointer. Public templates use
non-template bridges instead of accessing its storage; installed headers need
only the `include/` tree, not private headers copied from `src/`. The public API
is source-compatible, but the object layout changed: rebuild both libraries and
consumers together. The existing `EntityObserver`, `WindowObserver` and
`ViewObserver` value types remain available for compatibility; runtime-owned
containers and complete state stay private.

## Window resources and callbacks

- Live runtime records have stable addresses across creation of other windows.
  `window_runtime_records()` is a read-only contiguous **snapshot**, not the live
  store. Do not retain its span across a later snapshot query. A live record pointer
  must not be used after its window has been closed and reclaimed.
- Closing a window marks it inactive immediately. A view or element tree executing
  a callback remains alive until that callback returns. Native windows and owned
  renderers retire at a safe event-loop point; renderer resources release before
  the corresponding native window.
- `run_app` owns the renderers returned by `AppRendererFactory`. The low-level
  `WindowRuntime::RendererFactory` still returns borrowed pointers: its caller
  remains responsible for their lifetime. Ownership diagnostics distinguish these
  cases. Failed startup clears runtime pointers before destroying the native root.
- `Window` and `WindowRuntimeContext` redraw requests target their window ID.
  Requests made while rendering schedule another frame instead of being consumed
  by completion of the current frame. A manually constructed legacy context that
  omits its ID still addresses the root; a stale nonzero child ID never falls
  back to the root.
- Deferred, timer, task/group and I/O callbacks created through a window context
  retain its ID, not a borrowed context reference. Closed-window callbacks are
  skipped while task/I/O terminal bookkeeping still completes. Window-owned
  repeating timers are cancelled on close rather than waking indefinitely.

## Recoverable rendering

`ErrorCode::frame_retry_required` means the current frame was not delivered but
the rendering surface recovered. The runtime requests another frame for the same
window; it does not invoke the fatal-error path or quit the application. Other
rendering errors retain their existing failure semantics. Error messages are not
parsed to decide whether recovery is possible.

An upload staged before Vulkan surface acquisition is not necessarily submitted.
Unsubmitted image staging survives a retry until it is submitted, invalidated,
replaced, or made incompatible by a texture allocation change. Pending images
are protected from idle-cache eviction. Submitted staging retires only after the
previous frame is idle.

## Popup capability boundaries

Vulkan is not preemptively routed to an overlay. Independent popups are attempted
when the platform and renderer support them; missing capability or a real native
creation, renderer-initialization, or placement failure uses the overlay fallback.

On Windows, the platform probes only whether a DirectComposition host can be
created. This host needs no D3D11 rendering device. The renderer separately owns
the selected adapter, GPU interop, and presentation bridge. A default-adapter
D3D11 smoke probe must not determine platform-wide popup support. DirectComposition
is an internal Windows implementation detail, not an application window mode.

## Async resource retirement

Completed or cancelled task and I/O records can retain terminal status for handle
queries, but they no longer retain executable completion closures. Callback
captures release outside registry/state mutexes, including recursive task-group
cancellation, so capture destructors may safely perform supported handle queries.
Background work already executing still owns the captures it needs until it exits.
Terminal metadata compaction and task-handle lifetime beyond the runtime are not
introduced by this change.

`TaskGroup` is a UI-thread RAII facade. An opaque weak lifetime token makes its
late destruction or move safe after the runtime has shut down; this is not
concurrent-runtime-destruction support. Shutdown rejects new I/O registrations
and releases cross-thread queue captures outside their mutex too.

Actual runtime destruction cancels tasks and joins workers, then releases views,
element trees, globals, entities, observers, actions, animations and callback
captures while task metadata is still readable. Containers are detached before
user destructors run, and reentrant additions are drained before TaskGroup tokens
are detached. Terminal shutdown rejects new task, timer and deferred work; an
ordinary completed `run()` remains distinct and can be followed by another run.

After token detachment, TaskGroup queries return zero counts/false states and
task or child-group creation yields invalid handles. Its destructor and move
assignment are safe no-ops against the expired runtime. This protection does
not extend raw TaskHandle queries beyond the runtime's lifetime.

## Validation scope

Registered tests cover real Win32 registration and host creation, runtime callback
reentrancy and resource lifetime, recoverable frame scheduling, and Vulkan upload
retention. Metal readback uses non-framebuffer-only drawables and has a native
regression test; Windows or WSL runs do not substitute for running that test on a
Mac. See the dated acceptance record under
`.planning/2026-11-23-logic-cleanup/` for actual results and remaining limitations.
