#pragma once

#include "cgpui/renderer/renderer.hpp"
#include "renderer_pixel_test_backend.hpp"
#include "renderer_pixel_test_window.hpp"
#include "renderer_pixel_test_probes.hpp"

#include <optional>
#include <utility>

namespace cgpui_test {
[[nodiscard]] cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_pixel_capture(
    cgpui::Renderer& renderer);
[[nodiscard]] std::optional<cgpui::RendererFramePixels> completed_pixel_capture(
    cgpui::Renderer& renderer);

// Draws are logical pixels; the captured framebuffer is device pixels.
template <typename Paint>
std::optional<cgpui::RendererFramePixels> capture_pixels_at_scale(
    RendererPixelTestWindow& window, cgpui::DpiScale scale,
    cgpui::Color clear, Paint&& paint) {
  auto renderer = create_pixel_test_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = scale});
  if (!renderer) return std::nullopt;
  auto frame = begin_pixel_capture(**renderer);
  if (!frame) return std::nullopt;
  (*frame)->clear(clear);
  std::forward<Paint>(paint)(**frame);
  if (!pixel_test_ok((*frame)->present(), "present")) return std::nullopt;
  return completed_pixel_capture(**renderer);
}

template <typename Paint>
std::optional<cgpui::RendererFramePixels> capture_pixels(
    RendererPixelTestWindow& window, cgpui::Color clear, Paint&& paint) {
  return capture_pixels_at_scale(window, {1.0F}, clear, std::forward<Paint>(paint));
}
} // namespace cgpui_test
