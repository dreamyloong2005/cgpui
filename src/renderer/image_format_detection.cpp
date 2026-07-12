#include "image_format_detection_internal.hpp"

#include <algorithm>
#include <array>

namespace cgpui::detail {

EncodedImageFormat detect_encoded_image_format(
    std::span<const std::uint8_t> bytes) {
  constexpr std::array<std::uint8_t, 8> png_signature{
      0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
  if (bytes.size() >= png_signature.size() &&
      std::equal(png_signature.begin(), png_signature.end(), bytes.begin())) {
    return EncodedImageFormat::png;
  }
  if (bytes.size() >= 3 && bytes[0] == 0xff && bytes[1] == 0xd8 &&
      bytes[2] == 0xff) {
    return EncodedImageFormat::jpeg;
  }
  if (bytes.size() >= 6 && bytes[0] == 'G' && bytes[1] == 'I' &&
      bytes[2] == 'F' && bytes[3] == '8' &&
      (bytes[4] == '7' || bytes[4] == '9') && bytes[5] == 'a') {
    return EncodedImageFormat::gif;
  }
  return EncodedImageFormat::unknown;
}

} // namespace cgpui::detail
