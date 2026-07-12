#include "cgpui/renderer/image_decode.hpp"

#include "image_format_detection_internal.hpp"

#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_GIF
#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <limits>
#include <memory>

namespace cgpui {
namespace {

bool exceeds_decode_limits(
    int width,
    int height,
    const ImageDecodeLimits& limits,
    std::size_t& byte_size) {
  if (width <= 0 || height <= 0 ||
      static_cast<std::uint64_t>(width) > limits.max_width ||
      static_cast<std::uint64_t>(height) > limits.max_height) {
    return true;
  }
  const auto pixels = static_cast<std::uint64_t>(width) *
                      static_cast<std::uint64_t>(height);
  const auto bytes = pixels * 4U;
  if (pixels > limits.max_pixels || bytes > limits.max_decoded_bytes ||
      bytes > std::numeric_limits<std::size_t>::max() ||
      static_cast<std::uint64_t>(width) * 4U >
          std::numeric_limits<std::uint32_t>::max()) {
    return true;
  }
  byte_size = static_cast<std::size_t>(bytes);
  return false;
}

} // namespace

bool ImageDecodeResult::ready() const {
  return status == ImageDecodeStatus::ready;
}

ImageDecodeResult decode_image(
    std::span<const std::uint8_t> encoded,
    ImageDecodeLimits limits) {
  ImageDecodeResult result;
  if (encoded.empty()) return result;
  result.source_format = detail::detect_encoded_image_format(encoded);
  if (result.source_format == EncodedImageFormat::unknown) {
    result.status = ImageDecodeStatus::unsupported_format;
    return result;
  }
  if (result.source_format == EncodedImageFormat::gif) {
    result.status = ImageDecodeStatus::unsupported_format;
    return result;
  }
  if (encoded.size() > static_cast<std::size_t>(
                           std::numeric_limits<int>::max())) {
    result.status = ImageDecodeStatus::exceeds_limits;
    return result;
  }

  int width = 0;
  int height = 0;
  int source_channels = 0;
  const auto* data = reinterpret_cast<const stbi_uc*>(encoded.data());
  const int length = static_cast<int>(encoded.size());
  if (stbi_info_from_memory(
          data, length, &width, &height, &source_channels) == 0) {
    result.status = ImageDecodeStatus::invalid_data;
    return result;
  }
  std::size_t byte_size = 0;
  if (exceeds_decode_limits(width, height, limits, byte_size)) {
    result.status = ImageDecodeStatus::exceeds_limits;
    return result;
  }

  int decoded_width = 0;
  int decoded_height = 0;
  const std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> decoded(
      stbi_load_from_memory(
          data, length, &decoded_width, &decoded_height, nullptr, 4),
      &stbi_image_free);
  if (!decoded || decoded_width != width || decoded_height != height) {
    result.status = ImageDecodeStatus::decode_failed;
    return result;
  }
  result.bitmap = DecodedImageBitmap{
      .width = static_cast<std::uint32_t>(width),
      .height = static_cast<std::uint32_t>(height),
      .stride = static_cast<std::uint32_t>(width) * 4U,
      .format = ImageFormat::rgba8_unorm,
      .pixels = std::vector<std::uint8_t>(
          decoded.get(), decoded.get() + byte_size),
  };
  result.status = ImageDecodeStatus::ready;
  return result;
}

} // namespace cgpui
