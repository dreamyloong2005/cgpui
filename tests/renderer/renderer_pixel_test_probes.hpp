#pragma once

#include "cgpui/renderer/renderer_frame_pixels.hpp"

#include <array>
#include <cstdint>

namespace cgpui_test {
[[nodiscard]] std::uint8_t encoded_channel(
    float linear, cgpui::RendererFramePixelEncoding encoding);
[[nodiscard]] bool pixel_near(const cgpui::RendererFramePixels& pixels,
    std::uint32_t x, std::uint32_t y, std::array<std::uint8_t, 4> expected,
    std::uint8_t tolerance = 1);
} // namespace cgpui_test
