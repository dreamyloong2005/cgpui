#include "renderer_text_pixel_shape_support.hpp"

#include <algorithm>

namespace cgpui_test {
bool TextInkShape::shaped() const {
  return right > left && bottom > top && pixels >= 20 &&
      pixels < static_cast<std::size_t>(right - left) * (bottom - top) * 85 / 100;
}

TextInkShape text_ink_shape(const cgpui::RendererFramePixels& pixels) {
  TextInkShape ink{.left = pixels.width, .top = pixels.height};
  for (std::uint32_t y = 0; y < pixels.height; ++y) {
    for (std::uint32_t x = 0; x < pixels.width; ++x) {
      const auto color = pixels.pixel_rgba8(x, y);
      if (color && (*color)[0] > 16 && (*color)[1] > 16 && (*color)[2] > 16) {
        ++ink.pixels;
        ink.left = (std::min)(ink.left, x); ink.top = (std::min)(ink.top, y);
        ink.right = (std::max)(ink.right, x + 1); ink.bottom = (std::max)(ink.bottom, y + 1);
      }
    }
  }
  return ink;
}

} // namespace cgpui_test
