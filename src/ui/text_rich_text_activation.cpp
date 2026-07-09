#include "cgpui/ui/text_rich_text_activation.hpp"

namespace cgpui {

bool rich_text_pointer_button_can_activate_link(const PointerButton& button) {
  return button.button == MouseButton::left && !button.pressed &&
         button.click_count == 1;
}

std::optional<RichTextLinkActivation> rich_text_link_activation_at_point(
    std::span<const RichTextRun> runs,
    const TextMeasurement& measurement,
    Rect bounds,
    const PointerButton& button) {
  if (!rich_text_pointer_button_can_activate_link(button)) {
    return std::nullopt;
  }

  const std::optional<RichTextLinkPointHit> hit = rich_text_link_at_point(
      runs,
      measurement,
      bounds,
      button.position);
  if (!hit.has_value() || !hit->text_hit.inside_bounds) {
    return std::nullopt;
  }

  return RichTextLinkActivation{
      .link_id = hit->link_hit.link_id,
      .run_index = hit->link_hit.run_index,
      .byte_start = hit->link_hit.byte_start,
      .byte_end = hit->link_hit.byte_end,
      .text_hit = hit->text_hit,
      .button = button.button,
      .click_count = button.click_count,
  };
}

} // namespace cgpui
