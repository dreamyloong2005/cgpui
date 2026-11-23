#pragma once

#include "dx12_ordinary_test_support.hpp"

namespace cgpui_test {

// Alpha in the captured GPU image must not change when presentation changes
// between opaque and transparent. Only the desktop composition differs.
inline bool ordinary_draw_bands(cgpui::Renderer& renderer,
                                const cgpui::PlatformWindow& window, bool blue) {
  const auto state = window.state();
  const float width = state.framebuffer_size.width / state.scale.value;
  const float height = state.framebuffer_size.height / state.scale.value;
  auto frame = renderer.begin_frame();
  if (!frame) return false;
  (*frame)->clear({0, 0, 0, 0});
  (*frame)->draw_rect({.rect = {{0, 0}, {width / 3, height}},
      .color = blue ? cgpui::Color{0, 0, 1, 0.5F} : cgpui::Color{1, 0, 0, 0.5F}});
  (*frame)->draw_rect({.rect = {{width * 2 / 3, 0}, {width / 3, height}},
      .color = blue ? cgpui::Color{0, 0, 1, 1} : cgpui::Color{1, 0, 0, 1}});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present()) return false;
  const auto* pixels = renderer.last_frame_pixels();
  if (!pixels || !pixels->valid() || pixels->width != unsigned(state.framebuffer_size.width) ||
      pixels->height != unsigned(state.framebuffer_size.height)) return false;
  const auto half = blue ? std::array<int, 4>{0, 0, 128, 128} : std::array<int, 4>{128, 0, 0, 128};
  const auto full = blue ? std::array<int, 4>{0, 0, 255, 255} : std::array<int, 4>{255, 0, 0, 255};
  return expect_texture_pixel(*pixels, pixels->width / 6, pixels->height / 2, half) &&
      expect_texture_pixel(*pixels, pixels->width / 2, pixels->height / 2, {0, 0, 0, 0}) &&
      expect_texture_pixel(*pixels, pixels->width * 5 / 6, pixels->height / 2, full);
}

inline bool ordinary_desktop_bands(const cgpui::PlatformWindow& window, bool blue, bool alpha) {
  RECT client{};
  POINT origin{};
  const HWND hwnd = ordinary_hwnd(window);
  DWORD process = 0;
  GetWindowThreadProcessId(hwnd, &process);
  if (process != GetCurrentProcessId() || !GetClientRect(hwnd, &client) ||
      !ClientToScreen(hwnd, &origin) || client.right <= 0 || client.bottom <= 0) return false;
  const LONG y = origin.y + client.bottom / 2;
  const int green = alpha ? 127 : 0;
  return expect_screen_color(blue ? ScreenColor{0, green, 128} : ScreenColor{128, green, 0},
                            {origin.x + client.right / 6, y}) &&
      expect_screen_color(alpha ? ScreenColor{0, 255, 0} : ScreenColor{0, 0, 0},
                          {origin.x + client.right / 2, y}) &&
      expect_screen_color(blue ? ScreenColor{0, 0, 255} : ScreenColor{255, 0, 0},
                          {origin.x + client.right * 5 / 6, y});
}

} // namespace cgpui_test
