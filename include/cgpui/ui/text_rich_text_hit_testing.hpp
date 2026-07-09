#pragma once

#include "cgpui/ui/text_hit_testing.hpp"
#include "cgpui/ui/text_rich_text.hpp"

#include <optional>
#include <span>

namespace cgpui {

struct RichTextRunPointHit {
  TextHitTestResult text_hit;
  RichTextRunHit run_hit;
};

struct RichTextLinkPointHit {
  TextHitTestResult text_hit;
  RichTextLinkHit link_hit;
};

[[nodiscard]] std::optional<RichTextRunPointHit> rich_text_run_at_point(
    std::span<const RichTextRun> runs,
    const TextMeasurement& measurement,
    Rect bounds,
    Point point);

[[nodiscard]] std::optional<RichTextLinkPointHit> rich_text_link_at_point(
    std::span<const RichTextRun> runs,
    const TextMeasurement& measurement,
    Rect bounds,
    Point point);

} // namespace cgpui
