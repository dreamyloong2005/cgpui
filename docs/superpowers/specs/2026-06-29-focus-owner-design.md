# Focus Owner Design

## Goal

Add a small view identity and keyboard focus owner primitive so later routing work can tell which view owns keyboard focus instead of tracking only a window-level boolean.

## Scope

- Add a lightweight `ViewId` value type in the UI layer.
- Add `WindowRuntimeContext::view_id` for the currently dispatched root view.
- Add `ViewInputState::keyboard_focus_owner` as `std::optional<ViewId>`.
- Keep `ViewInputState::keyboard_focused`, but derive it from `keyboard_focus_owner == context.view_id`.
- Add owner-aware `WindowRuntime::request_keyboard_focus(ViewId)` and `WindowRuntime::release_keyboard_focus(ViewId)` overloads.
- Keep the existing no-argument focus APIs as root-view conveniences.
- Treat `release_keyboard_focus(ViewId)` from a non-owner as a no-op.
- Reset the keyboard focus owner at the start of each `WindowRuntime::run`.

## Non-Goals

- No view tree or child view allocation yet.
- No focus traversal or tab order.
- No event bubbling, cancellation, or retargeting.
- No platform IME/text-input activation behavior.
- No pointer capture owner in this slice.

## Testing

- Extend `window_runtime_test` to prove contexts expose a stable non-zero `ViewId`.
- Verify requesting focus stores that view id as `keyboard_focus_owner`.
- Verify later key and text input contexts report `keyboard_focused == true` for the owner.
- Verify a non-owner release does not clear focus.
- Verify releasing the owner clears both `keyboard_focus_owner` and `keyboard_focused`.
