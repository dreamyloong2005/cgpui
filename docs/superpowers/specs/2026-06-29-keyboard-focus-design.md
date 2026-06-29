# Keyboard Focus Design

## Goal

Add the smallest runtime-level keyboard focus primitive so a view can request focus during event handling and observe that focus state on later keyboard/text events.

## Scope

- Add `WindowRuntime::request_keyboard_focus()` and `WindowRuntime::release_keyboard_focus()`.
- Add `ViewInputState::keyboard_focused`.
- Store keyboard focus state in `WindowRuntime`.
- Expose keyboard focus state through `WindowRuntimeContext::input`.
- Reset keyboard focus state at the start of each `WindowRuntime::run`.

## Non-Goals

- No view identity or focus owner yet.
- No focus traversal yet.
- No event bubbling or cancellation yet.
- No platform IME/text-input activation behavior yet.

## Testing

- Extend `window_runtime_test` so a view requests keyboard focus during one event and sees focus state on a later keyboard event.
- Add a release-path assertion that keyboard focus can be cleared from event handling.
- Keep existing pointer capture and input state behavior unchanged.
