#include "cgpui/ui/text_rich_text_inline_image.hpp"

#include <algorithm>
#include <optional>

namespace cgpui {
namespace {

bool inline_image_span_has_valid_size(
    const RichTextInlineImageSpan& image) {
  return image.image_id.value != 0U && image.logical_size.width > 0.0F &&
         image.logical_size.height > 0.0F;
}

std::optional<RichTextInlineImageRun> clipped_inline_image_span(
    const RichTextInlineImageSpan& image,
    std::size_t text_length) {
  if (!inline_image_span_has_valid_size(image) ||
      image.byte_start > image.byte_end || image.byte_start > text_length) {
    return std::nullopt;
  }

  return RichTextInlineImageRun{
      .image_id = image.image_id,
      .byte_start = image.byte_start,
      .byte_end = std::min(image.byte_end, text_length),
      .logical_size = image.logical_size,
      .baseline_offset = image.baseline_offset,
  };
}

void sort_inline_image_runs(std::vector<RichTextInlineImageRun>& runs) {
  std::sort(
      runs.begin(),
      runs.end(),
      [](const RichTextInlineImageRun& left,
         const RichTextInlineImageRun& right) {
        if (left.byte_start != right.byte_start) {
          return left.byte_start < right.byte_start;
        }
        if (left.byte_end != right.byte_end) {
          return left.byte_end < right.byte_end;
        }
        return left.image_id.value < right.image_id.value;
      });
}

} // namespace

void build_rich_text_inline_image_runs(
    std::string_view text,
    std::span<const RichTextInlineImageSpan> images,
    std::vector<RichTextInlineImageRun>& runs) {
  runs.clear();
  runs.reserve(images.size());
  for (const RichTextInlineImageSpan& image : images) {
    const std::optional<RichTextInlineImageRun> run =
        clipped_inline_image_span(image, text.size());
    if (run.has_value()) {
      runs.push_back(*run);
    }
  }
  sort_inline_image_runs(runs);
}

std::vector<RichTextInlineImageRun> build_rich_text_inline_image_runs(
    std::string_view text,
    std::span<const RichTextInlineImageSpan> images) {
  std::vector<RichTextInlineImageRun> runs;
  build_rich_text_inline_image_runs(text, images, runs);
  return runs;
}

} // namespace cgpui
