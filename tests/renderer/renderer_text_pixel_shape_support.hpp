#pragma once

#include "cgpui/renderer/renderer_frame_pixels.hpp"

namespace cgpui_test {
struct TextInkShape {
  std::size_t pixels = 0;
  std::uint32_t left = 0, top = 0, right = 0, bottom = 0;
  [[nodiscard]] bool shaped() const;
};
[[nodiscard]] TextInkShape text_ink_shape(const cgpui::RendererFramePixels& pixels);
} // namespace cgpui_test
