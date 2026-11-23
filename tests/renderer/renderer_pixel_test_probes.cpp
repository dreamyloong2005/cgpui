#include "renderer_pixel_test_probes.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui_test {
std::uint8_t encoded_channel(
    float linear,
    cgpui::RendererFramePixelEncoding encoding) {
  const float encoded =
      encoding == cgpui::RendererFramePixelEncoding::srgb
          ? (linear <= 0.0031308F
                 ? linear * 12.92F
                 : 1.055F * std::pow(linear, 1.0F / 2.4F) - 0.055F)
          : linear;
  return static_cast<std::uint8_t>(
      std::clamp(encoded, 0.0F, 1.0F) * 255.0F + 0.5F);
}

bool pixel_near(
    const cgpui::RendererFramePixels& pixels,
    std::uint32_t x,
    std::uint32_t y,
    std::array<std::uint8_t, 4> expected,
    std::uint8_t tolerance) {
  const auto actual = pixels.pixel_rgba8(x, y);
  if (!actual.has_value()) {
    return false;
  }
  for (std::size_t index = 0; index < actual->size(); ++index) {
    if (std::abs(static_cast<int>((*actual)[index]) - expected[index]) >
        tolerance) {
      return false;
    }
  }
  return true;
}

} // namespace cgpui_test
