#pragma once

#include "cgpui/renderer/gif_decode.hpp"

namespace cgpui::detail {

struct GifMetadataScan {
  GifDecodeStatus status = GifDecodeStatus::invalid_data;
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  GifLoopBehavior loop;
  std::vector<std::uint32_t> frame_durations_ms;
  std::size_t total_decoded_bytes = 0;

  [[nodiscard]] bool ready() const {
    return status == GifDecodeStatus::ready;
  }
};

[[nodiscard]] GifMetadataScan scan_gif_metadata(
    std::span<const std::uint8_t> encoded,
    const GifDecodeLimits& limits);

} // namespace cgpui::detail
