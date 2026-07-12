#pragma once

#include "cgpui/renderer/image_decode.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace cgpui {

enum class GifDecodeStatus {
  ready,
  empty_input,
  unsupported_format,
  invalid_data,
  exceeds_limits,
  decode_failed,
};

struct GifLoopBehavior {
  bool infinite = false;
  std::uint16_t repeat_count = 0;
};

struct DecodedGifFrame {
  std::uint32_t duration_ms = 0;
  DecodedImageBitmap bitmap;
};

struct DecodedGif {
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  GifLoopBehavior loop;
  std::vector<DecodedGifFrame> frames;
};

struct GifDecodeLimits {
  ImageDecodeLimits image;
  std::size_t max_frames = 512;
  std::size_t max_total_decoded_bytes = 512U * 1024U * 1024U;
};

struct GifDecodeResult {
  GifDecodeStatus status = GifDecodeStatus::empty_input;
  DecodedGif gif;

  [[nodiscard]] bool ready() const;
};

[[nodiscard]] GifDecodeResult decode_gif(
    std::span<const std::uint8_t> encoded,
    GifDecodeLimits limits = {});

} // namespace cgpui
