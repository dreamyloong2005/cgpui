# Keyboard Focus Routing Design

## Goal

Route keyboard and text-input events to the element that owns keyboard focus by
attaching that `ElementId` to the active `EventRoute`.

## Decisions

- Keep the existing view-level keyboard focus API intact.
- Add element-level overloads:
  - `WindowRuntime::request_keyboard_focus(ElementId)`
  - `WindowRuntime::release_keyboard_focus(ElementId)`
- `ViewInputState` exposes `keyboard_focus_element_owner` for handlers and
  tests.
- Keyboard and text input events use the focused element as
  `EventRoute::target_element_id` while that owner is active.
- Releasing with a different `ElementId` is ignored; releasing with the owner
  restores keyboard/text routes without an element target.

## Non-Goals

- No element event handler dispatch yet.
- No focus validation against a live element tree.
- No key binding, action dispatch, or text-model mutation in this slice.
