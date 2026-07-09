#include "cgpui/ui/text_rich_text.hpp"

#include <algorithm>

namespace cgpui {
namespace {

bool color_equals(const Color& left, const Color& right) {
  return left.r == right.r && left.g == right.g && left.b == right.b &&
         left.a == right.a;
}

bool optional_color_equals(
    const std::optional<Color>& left,
    const std::optional<Color>& right) {
  if (left.has_value() != right.has_value()) {
    return false;
  }
  if (!left.has_value()) {
    return true;
  }
  return color_equals(*left, *right);
}

std::size_t clipped_span_start(
    const RichTextSpan& span,
    std::size_t text_length) {
  return std::min(span.byte_start, text_length);
}

std::size_t clipped_span_end(
    const RichTextSpan& span,
    std::size_t text_length) {
  return std::min(span.byte_end, text_length);
}

bool span_covers_segment(
    const RichTextSpan& span,
    std::size_t segment_start,
    std::size_t segment_end,
    std::size_t text_length) {
  const std::size_t start = clipped_span_start(span, text_length);
  const std::size_t end = clipped_span_end(span, text_length);
  return start < end && start <= segment_start && segment_end <= end;
}

void append_endpoint(
    std::vector<std::size_t>& endpoints,
    std::size_t value) {
  endpoints.push_back(value);
}

void sort_unique_endpoints(std::vector<std::size_t>& endpoints) {
  std::sort(endpoints.begin(), endpoints.end());
  endpoints.erase(
      std::unique(endpoints.begin(), endpoints.end()),
      endpoints.end());
}

RichTextAttributes attributes_for_segment(
    std::span<const RichTextSpan> spans,
    std::size_t segment_start,
    std::size_t segment_end,
    std::size_t text_length) {
  RichTextAttributes attributes;
  for (const RichTextSpan& span : spans) {
    if (span_covers_segment(span, segment_start, segment_end, text_length)) {
      attributes = merge_rich_text_attributes(attributes, span.attributes);
    }
  }
  return attributes;
}

void append_run(
    std::vector<RichTextRun>& runs,
    RichTextRun run) {
  if (run.byte_start == run.byte_end) {
    return;
  }
  if (!runs.empty() && runs.back().byte_end == run.byte_start &&
      runs.back().attributes == run.attributes) {
    runs.back().byte_end = run.byte_end;
    return;
  }
  runs.push_back(run);
}

bool run_contains_byte_offset(
    const RichTextRun& run,
    std::size_t byte_offset) {
  return run.byte_start <= byte_offset && byte_offset < run.byte_end;
}

} // namespace

bool operator==(
    const RichTextAttributes& left,
    const RichTextAttributes& right) {
  return optional_color_equals(left.foreground, right.foreground) &&
         optional_color_equals(left.background, right.background) &&
         left.decorations == right.decorations && left.link_id == right.link_id;
}

RichTextAttributes merge_rich_text_attributes(
    RichTextAttributes base,
    const RichTextAttributes& overlay) {
  if (overlay.foreground.has_value()) {
    base.foreground = overlay.foreground;
  }
  if (overlay.background.has_value()) {
    base.background = overlay.background;
  }
  base.decorations = base.decorations | overlay.decorations;
  if (overlay.link_id.has_value()) {
    base.link_id = overlay.link_id;
  }
  return base;
}

void build_rich_text_runs(
    std::string_view text,
    std::span<const RichTextSpan> spans,
    std::vector<RichTextRun>& runs,
    RichTextRunBuildScratch& scratch) {
  runs.clear();
  scratch.endpoints.clear();
  if (text.empty()) {
    return;
  }

  std::vector<std::size_t>& endpoints = scratch.endpoints;
  endpoints.reserve(2 + spans.size() * 2);
  append_endpoint(endpoints, 0);
  append_endpoint(endpoints, text.size());
  for (const RichTextSpan& span : spans) {
    const std::size_t start = clipped_span_start(span, text.size());
    const std::size_t end = clipped_span_end(span, text.size());
    if (start < end) {
      append_endpoint(endpoints, start);
      append_endpoint(endpoints, end);
    }
  }

  sort_unique_endpoints(endpoints);
  runs.reserve(endpoints.size() - 1);
  for (std::size_t index = 1; index < endpoints.size(); ++index) {
    const std::size_t start = endpoints[index - 1];
    const std::size_t end = endpoints[index];
    append_run(
        runs,
        RichTextRun{
            .byte_start = start,
            .byte_end = end,
            .attributes =
                attributes_for_segment(spans, start, end, text.size()),
        });
  }
}

void build_rich_text_runs(
    std::string_view text,
    std::span<const RichTextSpan> spans,
    std::vector<RichTextRun>& runs) {
  RichTextRunBuildScratch scratch;
  build_rich_text_runs(text, spans, runs, scratch);
}

std::vector<RichTextRun> build_rich_text_runs(
    std::string_view text,
    std::span<const RichTextSpan> spans) {
  std::vector<RichTextRun> runs;
  build_rich_text_runs(text, spans, runs);
  return runs;
}

std::optional<RichTextRunHit> rich_text_run_at_byte_offset(
    std::span<const RichTextRun> runs,
    std::size_t byte_offset) {
  for (std::size_t index = 0; index < runs.size(); ++index) {
    const RichTextRun& run = runs[index];
    if (run_contains_byte_offset(run, byte_offset)) {
      return RichTextRunHit{
          .run_index = index,
          .byte_start = run.byte_start,
          .byte_end = run.byte_end,
          .attributes = run.attributes,
      };
    }
  }
  return std::nullopt;
}

std::optional<RichTextLinkHit> rich_text_link_at_byte_offset(
    std::span<const RichTextRun> runs,
    std::size_t byte_offset) {
  const std::optional<RichTextRunHit> run_hit =
      rich_text_run_at_byte_offset(runs, byte_offset);
  if (!run_hit.has_value() || !run_hit->attributes.link_id.has_value()) {
    return std::nullopt;
  }

  return RichTextLinkHit{
      .link_id = *run_hit->attributes.link_id,
      .run_index = run_hit->run_index,
      .byte_start = run_hit->byte_start,
      .byte_end = run_hit->byte_end,
  };
}

} // namespace cgpui
