#pragma once

#include "cgpui/renderer/renderer_types.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

namespace cgpui {

enum class EncodedImageFormat {
  unknown,
  png,
  jpeg,
  gif,
};

enum class ImageDecodeStatus {
  ready,
  empty_input,
  unsupported_format,
  invalid_data,
  exceeds_limits,
  decode_failed,
};

struct ImageDecodeLimits {
  std::uint32_t max_width = 16384;
  std::uint32_t max_height = 16384;
  std::size_t max_pixels = 64U * 1024U * 1024U;
  std::size_t max_decoded_bytes = 256U * 1024U * 1024U;
};

struct ImageDecodeResult {
  ImageDecodeStatus status = ImageDecodeStatus::empty_input;
  EncodedImageFormat source_format = EncodedImageFormat::unknown;
  DecodedImageBitmap bitmap;

  [[nodiscard]] bool ready() const;
};

[[nodiscard]] ImageDecodeResult decode_image(
    std::span<const std::uint8_t> encoded,
    ImageDecodeLimits limits = {});

} // namespace cgpui
