# View Input State Design

## Goal

Expose a small runtime-maintained input snapshot to `View::handle_event` so views can react to events with current focus and pointer context.

## Scope

- Add `ViewInputState` to the UI layer.
- Store it in `WindowRuntime`.
- Expose it as `WindowRuntimeContext::input`.
- Update it before dispatching these events to the view:
  - `WindowFocused`: updates `input.focused`.
  - `PointerMoved`, `PointerButton`, and `PointerScrolled`: update `input.pointer_position`.

## Non-Goals

- No view tree hit testing yet.
- No pointer capture owner yet.
- No keyboard focus owner yet.
- No event bubbling or cancellation yet.

## Testing

- Extend `window_runtime_test` so the recording view observes the input state from context.
- Verify pointer and focus state are updated before the event reaches the view.
- Verify lifecycle events still stay owned by `WindowRuntime`.
