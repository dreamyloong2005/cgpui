#include "cgpui/ui/text_rich_text_hit_testing.hpp"

namespace cgpui {

std::optional<RichTextRunPointHit> rich_text_run_at_point(
    std::span<const RichTextRun> runs,
    const TextMeasurement& measurement,
    Rect bounds,
    Point point) {
  const TextHitTestResult text_hit =
      hit_test_text_position(measurement, bounds, point);
  const std::optional<RichTextRunHit> run_hit =
      rich_text_run_at_byte_offset(runs, text_hit.byte_offset);
  if (!run_hit.has_value()) {
    return std::nullopt;
  }
  return RichTextRunPointHit{
      .text_hit = text_hit,
      .run_hit = *run_hit,
  };
}

std::optional<RichTextLinkPointHit> rich_text_link_at_point(
    std::span<const RichTextRun> runs,
    const TextMeasurement& measurement,
    Rect bounds,
    Point point) {
  const TextHitTestResult text_hit =
      hit_test_text_position(measurement, bounds, point);
  const std::optional<RichTextLinkHit> link_hit =
      rich_text_link_at_byte_offset(runs, text_hit.byte_offset);
  if (!link_hit.has_value()) {
    return std::nullopt;
  }
  return RichTextLinkPointHit{
      .text_hit = text_hit,
      .link_hit = *link_hit,
  };
}

} // namespace cgpui
