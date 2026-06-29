# Pointer Capture Design

## Goal

Add the smallest runtime-level pointer capture primitive so a view can request capture during event handling and observe the capture state on later events.

## Scope

- Add `WindowRuntime::capture_pointer()` and `WindowRuntime::release_pointer()`.
- Add `ViewInputState::pointer_captured`.
- Store capture state in `WindowRuntime`.
- Expose capture state through `WindowRuntimeContext::input`.
- Reset capture state at the start of each `WindowRuntime::run`.

## Non-Goals

- No view identity or capture owner yet.
- No hit testing yet.
- No event bubbling or cancellation yet.
- No platform OS-level pointer confinement/capture yet.

## Testing

- Extend `window_runtime_test` so a view captures pointer on one pointer event and sees capture state on a following pointer event.
- Add a release-path assertion that capture can be cleared from event handling.
- Keep existing input state and redraw behavior unchanged.
