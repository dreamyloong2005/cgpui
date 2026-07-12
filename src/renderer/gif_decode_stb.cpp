#include "cgpui/renderer/gif_decode.hpp"

#include "gif_metadata_internal.hpp"

#include <stb_image.h>

#include <limits>
#include <memory>

namespace cgpui {

bool GifDecodeResult::ready() const {
  return status == GifDecodeStatus::ready;
}

GifDecodeResult decode_gif(
    std::span<const std::uint8_t> encoded,
    GifDecodeLimits limits) {
  GifDecodeResult result;
  const detail::GifMetadataScan metadata =
      detail::scan_gif_metadata(encoded, limits);
  if (!metadata.ready()) {
    result.status = metadata.status;
    return result;
  }
  if (encoded.size() > static_cast<std::size_t>(
                           std::numeric_limits<int>::max())) {
    result.status = GifDecodeStatus::exceeds_limits;
    return result;
  }

  int* delays = nullptr;
  int width = 0;
  int height = 0;
  int frame_count = 0;
  int source_channels = 0;
  const std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> decoded(
      stbi_load_gif_from_memory(
          encoded.data(), static_cast<int>(encoded.size()), &delays, &width,
          &height, &frame_count, &source_channels, 4),
      &stbi_image_free);
  const std::unique_ptr<int, decltype(&stbi_image_free)> delay_owner(
      delays, &stbi_image_free);
  if (!decoded || width != static_cast<int>(metadata.width) ||
      height != static_cast<int>(metadata.height) || frame_count <= 0 ||
      static_cast<std::size_t>(frame_count) !=
          metadata.frame_durations_ms.size()) {
    result.status = GifDecodeStatus::decode_failed;
    return result;
  }

  const std::size_t frame_bytes =
      metadata.total_decoded_bytes / metadata.frame_durations_ms.size();
  result.gif.width = metadata.width;
  result.gif.height = metadata.height;
  result.gif.loop = metadata.loop;
  result.gif.frames.reserve(metadata.frame_durations_ms.size());
  for (std::size_t index = 0; index < metadata.frame_durations_ms.size();
       ++index) {
    if (delays == nullptr || delays[index] < 0 ||
        static_cast<std::uint32_t>(delays[index]) !=
            metadata.frame_durations_ms[index]) {
      result.status = GifDecodeStatus::decode_failed;
      result.gif = {};
      return result;
    }
    const stbi_uc* first = decoded.get() + index * frame_bytes;
    result.gif.frames.push_back(DecodedGifFrame{
        .duration_ms = metadata.frame_durations_ms[index],
        .bitmap = DecodedImageBitmap{
            .width = metadata.width,
            .height = metadata.height,
            .stride = metadata.width * 4U,
            .format = ImageFormat::rgba8_unorm,
            .pixels = std::vector<std::uint8_t>(first, first + frame_bytes),
        },
    });
  }
  result.status = GifDecodeStatus::ready;
  return result;
}

} // namespace cgpui
