# Pointer Hit Routing Design

## Goal

Attach hit-tested element identity to pointer event routing while preserving the
current single-root-view dispatch model.

## Decisions

- `EventRoute` carries an optional `target_element_id` in addition to the
  existing root `ViewId` target.
- `WindowRuntime::set_element_root` installs a non-owning laid-out element root
  used only for hit testing pointer events.
- Pointer moved, button, and scroll events use their event position to hit-test
  the element root. Keyboard, text, focus, resize, redraw, and close events do
  not receive an element target in this slice.
- The root view still receives the event. Element-specific event handlers and
  delivery are intentionally left to later steps.

## Non-Goals

- No element event handler API.
- No pointer capture override; Step 15 owns that behavior.
- No element-tree traversal or layout scheduling.
