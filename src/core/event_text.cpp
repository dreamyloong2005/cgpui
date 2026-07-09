#include "cgpui/core/event_text.hpp"

#include <algorithm>
#include <limits>

namespace cgpui {

bool append_ime_preedit_style(
    ImeComposition& composition,
    ImePreeditStyleSpan style) {
  if (composition.preedit_style_count >=
      composition.preedit_styles.size()) {
    return false;
  }
  composition.preedit_styles[composition.preedit_style_count] = style;
  composition.preedit_style_count += 1;
  return true;
}

bool append_ime_default_preedit_style(ImeComposition& composition) {
  if (composition.phase != ImeCompositionPhase::update ||
      composition.text.empty()) {
    return false;
  }

  const auto max_length =
      static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max());
  return append_ime_preedit_style(
      composition,
      ImePreeditStyleSpan{
          .byte_offset = 0,
          .byte_length = static_cast<std::uint32_t>(
              std::min(composition.text.size(), max_length)),
          .kind = ImePreeditStyleKind::underline,
      });
}

} // namespace cgpui
