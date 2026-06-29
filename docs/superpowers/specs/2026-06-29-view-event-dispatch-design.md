# View Event Dispatch Design

## Goal

Let `WindowRuntime` deliver non-lifecycle platform input events to the active `View`, so UI code can react to focus, pointer, keyboard, and text input without each platform backend knowing about views.

## Scope

- Add a default no-op `View::handle_event(const PlatformEvent&, const WindowRuntimeContext&)` hook.
- Dispatch these events to the view: `WindowFocused`, `PointerMoved`, `PointerButton`, `PointerScrolled`, `KeyboardKey`, and `TextInput`.
- Keep `WindowCloseRequested`, `WindowResized`, and `WindowRedrawRequested` owned by `WindowRuntime`.
- Ignore events until both the platform window and renderer exist, because `WindowRuntimeContext` requires both.

## Testing

- Extend `window_runtime_test` with a recording view that proves input events reach `View::handle_event`.
- Add a runtime test showing a view can use the event context to request a redraw.
- Run targeted UI/header tests, then full Windows and WSL Arch Linux test suites.
