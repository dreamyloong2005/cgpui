# Pointer Capture Routing Design

## Goal

Let pointer capture override fresh hit testing so follow-up pointer events route
to the captured element until that same owner releases capture.

## Decisions

- Keep the existing view-level pointer capture API intact.
- Add element-level overloads:
  - `WindowRuntime::capture_pointer(ElementId)`
  - `WindowRuntime::release_pointer(ElementId)`
- `ViewInputState` exposes `pointer_capture_element_owner` for handlers and
  tests.
- While an element owner is captured, pointer moved/button/scroll routes use
  that owner as `EventRoute::target_element_id` instead of hit-testing the
  current pointer position.
- Releasing with a different `ElementId` is ignored; releasing with the owner
  restores normal hit-test routing.

## Non-Goals

- No element event handler dispatch yet.
- No implicit capture on button press.
- No capture validation against a live element tree.
