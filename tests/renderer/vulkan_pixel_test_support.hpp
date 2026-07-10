#pragma once

#include "cgpui/renderer/renderer.hpp"
#include "vulkan_pixel_test_window.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <utility>

namespace cgpui_test {

template <typename Paint>
std::optional<cgpui::RendererFramePixels> capture_pixels(
    VulkanPixelTestWindow& window,
    cgpui::Color clear,
    Paint&& paint) {
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(),
      .scale = cgpui::DpiScale{1.0F},
  });
  if (!renderer) {
    return std::nullopt;
  }
  auto frame = (*renderer)->begin_frame();
  if (!frame || !(*frame)->request_pixel_capture()) {
    return std::nullopt;
  }
  (*frame)->clear(clear);
  std::forward<Paint>(paint)(**frame);
  if (!(*frame)->present()) {
    return std::nullopt;
  }
  const cgpui::RendererFramePixels* pixels = (*renderer)->last_frame_pixels();
  return pixels != nullptr ? std::optional{*pixels} : std::nullopt;
}

inline std::uint8_t encoded_channel(
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

inline bool pixel_near(
    const cgpui::RendererFramePixels& pixels,
    std::uint32_t x,
    std::uint32_t y,
    std::array<std::uint8_t, 4> expected,
    std::uint8_t tolerance = 1) {
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
