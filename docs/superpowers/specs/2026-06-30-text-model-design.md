# Text Model Design

## Goal

Add a minimal editable UTF-8 text buffer with cursor movement and basic
deletion primitives.

## Decisions

- Add `cgpui/ui/text.hpp` as a focused header-only UI primitive.
- Store text as `std::string` and cursor as a byte offset.
- Cursor movement, backspace, and forward delete operate on UTF-8 codepoint
  boundaries by skipping continuation bytes.
- Inserted text is accepted as UTF-8 bytes and inserted at the current cursor.

## Non-Goals

- No selection model.
- No grapheme-cluster segmentation.
- No undo/redo or rich text spans.
- No runtime text-input routing until Step 20.
