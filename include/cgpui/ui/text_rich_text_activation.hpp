#pragma once

#include "cgpui/core/event_pointer.hpp"
#include "cgpui/ui/text_rich_text_hit_testing.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace cgpui {

struct RichTextLinkActivation {
  RichTextLinkId link_id;
  std::size_t run_index = 0;
  std::size_t byte_start = 0;
  std::size_t byte_end = 0;
  TextHitTestResult text_hit;
  MouseButton button = MouseButton::left;
  std::uint8_t click_count = 1;
};

[[nodiscard]] bool rich_text_pointer_button_can_activate_link(
    const PointerButton& button);

[[nodiscard]] std::optional<RichTextLinkActivation>
rich_text_link_activation_at_point(
    std::span<const RichTextRun> runs,
    const TextMeasurement& measurement,
    Rect bounds,
    const PointerButton& button);

} // namespace cgpui
