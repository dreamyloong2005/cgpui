#pragma once

#include "cgpui/ui/paint.hpp"
#include "cgpui/ui/text_model.hpp"

#include <cstddef>
#include <string>

namespace cgpui {

// The text a TextElement shows while composing: the model text with the
// composition in place of the selection. Offsets are UTF-8 bytes.
struct TextCompositionDisplay {
  std::string text;
  std::size_t composition_start = 0;
  std::size_t caret = 0;
};

[[nodiscard]] TextCompositionDisplay text_composition_display(
    const TextModel& model);

// Draws the display text, one underline per clause (thin for input and
// converted clauses, thick for the selected clause), a highlight for a
// non-empty caret range, and the caret at the composition caret.
void paint_text_composition(
    PaintList& paint_list,
    Rect bounds,
    Color color,
    const TextModel& model,
    const FontDescriptor& font,
    float font_size);

} // namespace cgpui
