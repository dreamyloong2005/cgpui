#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"

#include <limits>

using namespace cgpui;
using namespace cgpui_test;

namespace {
BoxShadowDraw valid_shadow() {
  return {.rect = {{8, 8}, {16, 16}},
      .shadow = {.color = {1, 0, 0, 1}, .offset = {24, 0}}};
}

int check_valid(Renderer& renderer, const BoxShadowDraw& draw) {
  auto frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) return dx12_fail(10, "invalid shadow poisoned renderer");
  (*frame)->clear({1, 1, 1, 1});
  (*frame)->draw_box_shadow(draw);
  if (!(*frame)->request_pixel_capture()) return dx12_fail(11, "valid shadow capture failed");
  if (auto result = (*frame)->present(); !result) return dx12_fail(12, result.error().message.c_str());
  const auto* pixels = renderer.last_frame_pixels();
  return dx12_pixel(pixels, 40, 16, {255, 0, 0, 255}) &&
      dx12_pixel(pixels, 16, 16, {255, 255, 255, 255}) &&
      dx12_pixel(pixels, 52, 16, {255, 255, 255, 255})
      ? 0 : dx12_fail(13, "valid/sanitized shadow pixels differ");
}
}

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "validation shadow window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  const float nan = std::numeric_limits<float>::quiet_NaN();
  const float infinity = std::numeric_limits<float>::infinity();
  // Existing neutral/Vulkan policy normalizes nonfinite blur/spread to zero.
  // Retain that contract rather than introducing backend-specific rejection.
  for (const float value : {nan, infinity, -infinity}) {
    auto normalized = valid_shadow();
    normalized.shadow.blur_radius = value;
    normalized.shadow.spread_radius = value;
    if (const int result = check_valid(**renderer, normalized)) return result;
  }
  for (unsigned invalid_case = 0; invalid_case < 6; ++invalid_case) {
    auto invalid = valid_shadow();
    switch (invalid_case) {
      case 0: invalid.rect.origin.x = nan; break;
      case 1: invalid.rect.size.height = infinity; break;
      case 2: invalid.shadow.offset.x = nan; break;
      case 3: invalid.shadow.color.r = nan; break;
      case 4: invalid.shadow.color.a = infinity; break;
      case 5: invalid.clip_rect = Rect{{nan, 0}, {64, 64}}; break;
    }
    auto frame = cgpui_test::wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(3, "invalid shadow begin failed");
    (*frame)->draw_box_shadow(invalid);
    const auto result = (*frame)->present();
    if (result || result.error().code != ErrorCode::invalid_argument)
      return dx12_fail(4, "nonfinite shadow geometry accepted or wrong error");
    if (const int recovered = check_valid(**renderer, valid_shadow())) return recovered;
  }
  // An empty original box and a shape collapsed by negative spread draw nothing.
  auto frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!frame) return dx12_fail(5, "empty shadow begin failed");
  (*frame)->clear({1, 1, 1, 1});
  auto empty = valid_shadow();
  empty.rect.size.width = 0;
  (*frame)->draw_box_shadow(empty);
  empty = valid_shadow();
  empty.shadow.spread_radius = -20;
  (*frame)->draw_box_shadow(empty);
  if (!(*frame)->request_pixel_capture()) return dx12_fail(6, "empty shadow capture failed");
  if (auto result = (*frame)->present(); !result) return dx12_fail(7, result.error().message.c_str());
  return dx12_pixel((*renderer)->last_frame_pixels(), 40, 16, {255, 255, 255, 255})
      ? 0 : dx12_fail(8, "empty shadow drew visible content");
}
