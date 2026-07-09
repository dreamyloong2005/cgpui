#include "cgpui/ui/text_rich_text.hpp"

#include <optional>
#include <span>
#include <vector>

namespace {

bool same_color(const cgpui::Color& left, const cgpui::Color& right) {
  return left.r == right.r && left.g == right.g && left.b == right.b &&
         left.a == right.a;
}

bool has_color(
    const std::optional<cgpui::Color>& value,
    const cgpui::Color& expected) {
  return value.has_value() && same_color(*value, expected);
}

int test_plain_text_builds_single_default_run() {
  std::vector<cgpui::RichTextRun> runs;
  cgpui::RichTextRunBuildScratch scratch;
  cgpui::build_rich_text_runs(
      "plain",
      std::span<const cgpui::RichTextSpan>{},
      runs,
      scratch);

  if (runs.size() != 1 || runs[0].byte_start != 0 ||
      runs[0].byte_end != 5 || runs[0].attributes.foreground.has_value() ||
      runs[0].attributes.background.has_value() ||
      runs[0].attributes.decorations != cgpui::RichTextDecoration::none ||
      runs[0].attributes.link_id.has_value() || scratch.endpoints.size() != 2) {
    return 1;
  }

  return 0;
}

int test_spans_clip_and_merge_attributes() {
  const cgpui::Color red = cgpui::rgb(255, 0, 0);
  const cgpui::Color blue = cgpui::rgb(0, 0, 255);
  std::vector<cgpui::RichTextSpan> spans{
      cgpui::RichTextSpan{
          .byte_start = 0,
          .byte_end = 5,
          .attributes = cgpui::RichTextAttributes{.foreground = red},
      },
      cgpui::RichTextSpan{
          .byte_start = 3,
          .byte_end = 12,
          .attributes = cgpui::RichTextAttributes{
              .background = blue,
              .decorations = cgpui::RichTextDecoration::underline,
              .link_id = cgpui::RichTextLinkId{7},
          },
      },
  };

  std::vector<cgpui::RichTextRun> runs;
  cgpui::RichTextRunBuildScratch scratch;
  cgpui::build_rich_text_runs("0123456789", spans, runs, scratch);

  if (runs.size() != 3 || scratch.endpoints.size() != 4) {
    return 10;
  }
  if (runs[0].byte_start != 0 || runs[0].byte_end != 3 ||
      !has_color(runs[0].attributes.foreground, red) ||
      runs[0].attributes.background.has_value() ||
      runs[0].attributes.decorations != cgpui::RichTextDecoration::none ||
      runs[0].attributes.link_id.has_value()) {
    return 11;
  }
  if (runs[1].byte_start != 3 || runs[1].byte_end != 5 ||
      !has_color(runs[1].attributes.foreground, red) ||
      !has_color(runs[1].attributes.background, blue) ||
      !cgpui::rich_text_has_decoration(
          runs[1].attributes.decorations,
          cgpui::RichTextDecoration::underline) ||
      !runs[1].attributes.link_id.has_value() ||
      runs[1].attributes.link_id->value != 7) {
    return 12;
  }
  if (runs[2].byte_start != 5 || runs[2].byte_end != 10 ||
      runs[2].attributes.foreground.has_value() ||
      !has_color(runs[2].attributes.background, blue) ||
      !cgpui::rich_text_has_decoration(
          runs[2].attributes.decorations,
          cgpui::RichTextDecoration::underline) ||
      !runs[2].attributes.link_id.has_value() ||
      runs[2].attributes.link_id->value != 7) {
    return 13;
  }

  return 0;
}

int test_empty_and_out_of_range_spans_are_ignored() {
  std::vector<cgpui::RichTextSpan> spans{
      cgpui::RichTextSpan{.byte_start = 2, .byte_end = 2},
      cgpui::RichTextSpan{.byte_start = 8, .byte_end = 10},
  };

  const std::vector<cgpui::RichTextRun> runs =
      cgpui::build_rich_text_runs("text", spans);

  if (runs.size() != 1 || runs[0].byte_start != 0 ||
      runs[0].byte_end != 4 ||
      runs[0].attributes != cgpui::RichTextAttributes{}) {
    return 20;
  }

  return 0;
}

int test_adjacent_equal_runs_coalesce() {
  const cgpui::Color green = cgpui::rgb(0, 255, 0);
  std::vector<cgpui::RichTextSpan> spans{
      cgpui::RichTextSpan{
          .byte_start = 0,
          .byte_end = 2,
          .attributes = cgpui::RichTextAttributes{.foreground = green},
      },
      cgpui::RichTextSpan{
          .byte_start = 2,
          .byte_end = 4,
          .attributes = cgpui::RichTextAttributes{.foreground = green},
      },
  };

  std::vector<cgpui::RichTextRun> runs;
  cgpui::build_rich_text_runs("text", spans, runs);

  if (runs.size() != 1 || runs[0].byte_start != 0 ||
      runs[0].byte_end != 4 ||
      !has_color(runs[0].attributes.foreground, green)) {
    return 30;
  }

  return 0;
}

int test_run_and_link_hits_use_half_open_byte_ranges() {
  std::vector<cgpui::RichTextRun> runs{
      cgpui::RichTextRun{
          .byte_start = 0,
          .byte_end = 4,
      },
      cgpui::RichTextRun{
          .byte_start = 4,
          .byte_end = 8,
          .attributes = cgpui::RichTextAttributes{
              .link_id = cgpui::RichTextLinkId{42},
          },
      },
  };

  const std::optional<cgpui::RichTextRunHit> first =
      cgpui::rich_text_run_at_byte_offset(runs, 0);
  const std::optional<cgpui::RichTextRunHit> boundary =
      cgpui::rich_text_run_at_byte_offset(runs, 4);
  const std::optional<cgpui::RichTextRunHit> end =
      cgpui::rich_text_run_at_byte_offset(runs, 8);
  if (!first.has_value() || first->run_index != 0 ||
      first->byte_start != 0 || first->byte_end != 4 ||
      !boundary.has_value() || boundary->run_index != 1 ||
      boundary->byte_start != 4 || boundary->byte_end != 8 ||
      end.has_value()) {
    return 40;
  }

  const std::optional<cgpui::RichTextLinkHit> no_link =
      cgpui::rich_text_link_at_byte_offset(runs, 2);
  const std::optional<cgpui::RichTextLinkHit> link =
      cgpui::rich_text_link_at_byte_offset(runs, 7);
  const std::optional<cgpui::RichTextLinkHit> link_end =
      cgpui::rich_text_link_at_byte_offset(runs, 8);
  if (no_link.has_value() || !link.has_value() ||
      link->link_id.value != 42 || link->run_index != 1 ||
      link->byte_start != 4 || link->byte_end != 8 ||
      link_end.has_value()) {
    return 41;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_plain_text_builds_single_default_run();
      result != 0) {
    return result;
  }
  if (const int result = test_spans_clip_and_merge_attributes(); result != 0) {
    return result;
  }
  if (const int result = test_empty_and_out_of_range_spans_are_ignored();
      result != 0) {
    return result;
  }
  if (const int result = test_adjacent_equal_runs_coalesce(); result != 0) {
    return result;
  }
  if (const int result = test_run_and_link_hits_use_half_open_byte_ranges();
      result != 0) {
    return result;
  }
  return 0;
}
