#pragma once

#include "cgpui/renderer/renderer_types.hpp"

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

namespace cgpui {

struct RichTextInlineImageSpan {
  ImageAssetId image_id;
  std::size_t byte_start = 0;
  std::size_t byte_end = 0;
  Size logical_size;
  float baseline_offset = 0.0F;
};

struct RichTextInlineImageRun {
  ImageAssetId image_id;
  std::size_t byte_start = 0;
  std::size_t byte_end = 0;
  Size logical_size;
  float baseline_offset = 0.0F;
};

void build_rich_text_inline_image_runs(
    std::string_view text,
    std::span<const RichTextInlineImageSpan> images,
    std::vector<RichTextInlineImageRun>& runs);

[[nodiscard]] std::vector<RichTextInlineImageRun>
build_rich_text_inline_image_runs(
    std::string_view text,
    std::span<const RichTextInlineImageSpan> images);

} // namespace cgpui
