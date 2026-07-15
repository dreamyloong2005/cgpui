#pragma once

#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

namespace cgpui_test {

class MetalPixelTestSurface {
 public:
  MetalPixelTestSurface()
      : application_(cgpui::create_platform_application()) {
    if (!application_) return;
    window_ = (*application_)->create_window(
        cgpui::WindowDescriptor{
            .title = "CGPUI Metal Pixel Test",
            .size = {64.0F, 64.0F}},
        [](const cgpui::PlatformEvent&) {});
  }

  [[nodiscard]] bool valid() const { return window_.has_value(); }

  [[nodiscard]] cgpui::NativeSurfaceHandle surface() const {
    return window_ ? (*window_)->native_surface() : cgpui::NativeSurfaceHandle{};
  }

 private:
  cgpui::Result<std::unique_ptr<cgpui::PlatformApplication>> application_;
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> window_ =
      std::unexpected(cgpui::Error{});
};

template <typename Paint>
std::optional<cgpui::RendererFramePixels> capture_metal_pixels(
    MetalPixelTestSurface& surface,
    cgpui::Color clear,
    Paint&& paint,
    const std::function<void(cgpui::Renderer&)>& inspect = {}) {
  if (!surface.valid()) return std::nullopt;
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = surface.surface(),
      .framebuffer_size = {64.0F, 64.0F},
      .scale = {1.0F}});
  if (!renderer) return std::nullopt;
  auto frame = (*renderer)->begin_frame();
  if (!frame || !(*frame)->request_pixel_capture()) return std::nullopt;
  (*frame)->clear(clear);
  std::forward<Paint>(paint)(**frame);
  if (!(*frame)->present()) return std::nullopt;
  if (inspect) inspect(**renderer);
  const cgpui::RendererFramePixels* pixels = (*renderer)->last_frame_pixels();
  return pixels == nullptr ? std::nullopt : std::optional{*pixels};
}

inline bool metal_pixel_near(
    const cgpui::RendererFramePixels& pixels,
    std::uint32_t x,
    std::uint32_t y,
    std::array<std::uint8_t, 4> expected,
    std::uint8_t tolerance = 3) {
  const auto actual = pixels.pixel_rgba8(x, y);
  if (!actual) return false;
  for (std::size_t channel = 0; channel < actual->size(); ++channel) {
    if (std::abs(static_cast<int>((*actual)[channel]) - expected[channel]) >
        tolerance) {
      return false;
    }
  }
  return true;
}

}  // namespace cgpui_test
