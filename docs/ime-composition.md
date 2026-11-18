# IME composition caret and clauses (C8)

`ImeComposition` updates carry the composition caret and per-clause
conversion state. The text input widget draws clause underlines, places the
caret at the composition caret, and IME candidate windows follow that caret.

## Public API

`include/cgpui/core/event_text.hpp`:

- `enum class ImeClauseStyle : std::uint8_t { input, selected_converted, converted, selected_unconverted }`
- `ImePreeditStyleSpan` gains `ImeClauseStyle clause = ImeClauseStyle::input`;
  `byte_offset` / `byte_length` are UTF-8 bytes into `ImeComposition::text`.
- `kImePreeditStyleSpanCapacity` is 16 (was 4). Spans beyond it merge into the
  last span.
- `ImeComposition::preedit_cursor_begin` / `preedit_cursor_end` (existing) are
  the caret range in UTF-8 bytes; both -1 hide the caret.

`include/cgpui/ui/text_composition.hpp` (new leaf):

- `struct TextCompositionClause { std::size_t byte_offset; std::size_t byte_length; ImeClauseStyle style; }`
- `struct TextComposition { std::string text; std::size_t caret_begin, caret_end; bool caret_visible; std::vector<TextCompositionClause> clauses; }`
- `TextComposition text_composition_from_ime(const ImeComposition&)` clamps
  offsets, snaps them to code point boundaries, drops empty clauses and falls
  back to one `input` clause over the whole text.
- `TextComposition text_composition_from_text(std::string)`: caret at the end,
  one input clause.

`TextModel` gains `composition()` and `set_composition(TextComposition)`;
`set_composition_text` keeps working (caret at the end, one input clause).
`TextElement` gains `display_text()` and `composition_start_rect(DpiScale)`;
while composing, `caret_rect` returns the composition caret.
`ImeCandidateRect` gains `std::optional<Rect> composition_rect`.

## Platforms

| Platform | Caret | Clauses |
| --- | --- | --- |
| Windows (IMM32) | `ImmGetCompositionStringW(GCS_CURSORPOS)`, UTF-16 units converted to UTF-8 bytes; a caret (begin == end); none reported means the end | `GCS_COMPCLAUSE` boundaries, each styled by the `GCS_COMPATTR` byte of its first unit; without clause data, runs of equal attributes; without either, one input clause |
| Linux (Wayland `zwp_text_input_v3`) | `preedit_string` `cursor_begin` / `cursor_end`, already UTF-8 bytes; -1 hides the caret | none in the protocol: one `input` span over the preedit |
| macOS | marked-text cursor (existing) | one `input` span (planned only) |

IMM32 attribute mapping: `ATTR_INPUT` and `ATTR_INPUT_ERROR` -> `input`,
`ATTR_TARGET_CONVERTED` -> `selected_converted`, `ATTR_CONVERTED` and
`ATTR_FIXEDCONVERTED` -> `converted`, `ATTR_TARGET_NOTCONVERTED` ->
`selected_unconverted`. UTF-16 offsets inside a surrogate pair map to the
start of the pair. The conversion is
`win32_ime_composition_update` in
`src/platform/win32/win32_ime_composition_detail.cpp`, free of IMM calls;
`src/platform/win32/win32_window_ime_detail.cpp` reads the IMM data.
The IMM context is released before events are dispatched.

## Widget rendering

While the model composes, `TextElement::paint` draws through
`src/ui/text_composition_paint.cpp`:

- the display text: model text with the composition in place of the
  selection (layout measures the same text, so it does not wrap mid-compose);
- one underline per clause and wrapped line, from the shaped grapheme
  advances (C7 metrics): x from the clause start to its end minus one device
  pixel (keeps adjacent clauses apart), y at baseline + the face's underline
  position snapped to a device row, height one underline thickness (at least
  one device pixel) for `input` / `converted`, twice that for the selected
  clauses;
- a selection highlight for a non-empty caret range;
- the caret at the composition caret, unless hidden.

## Candidate placement

`ImeTextInputPlacement::rect` is the caret before the composition (the
composition window anchor, `CFS_POINT`) and `candidate_rect` the composition
caret. Windows passes `candidate_rect` to `ImmSetCandidateWindow` with
`CFS_EXCLUDE`; Wayland sends it as `set_cursor_rectangle`. Placement is
re-applied after each event when either rect changes.

## Tests

- `win32_ime_composition_detail_test` (Windows): synthetic composition
  strings, attributes, clauses and cursor positions, surrogate pairs,
  clamping and the span capacity.
- `text_composition_paint_test`: normalization, underline x ranges and
  thickness at scale 1 and 1.5 against shaped advances, caret position,
  hidden caret, and runtime placement (candidates at the composition caret).
- `wayland_keyboard_test`: the test compositor's preedit gives one `input`
  clause with the v3 cursor.
- `ime_composition_structure_test`, `text_composition_header_cleanliness`.

## Not verified

Real IMEs cannot be driven automatically. Microsoft Pinyin (and any other
Windows IME) clause attributes, caret and candidate placement have not been
observed in a real window; neither has a Wayland input method. Clause
attributes come only from IMM32; TSF-only details (display attributes) are
not read.
