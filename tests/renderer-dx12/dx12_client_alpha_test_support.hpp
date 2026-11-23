#pragma once

#include "dx12_ordinary_pixels_support.hpp"
#include <cmath>

namespace cgpui_test {
inline cgpui::WindowDescriptor client_alpha_descriptor(const OrdinaryDesktop& desktop) {
  auto descriptor = desktop.descriptor(true);
  descriptor.chrome.title_bar = cgpui::WindowTitleBarMode::client;
  return descriptor;
}
struct ClientAlphaTopology {
  HWND hwnd = nullptr;
  RECT window{}, client{};
  POINT origin{};
  LONG_PTR style = 0, extended = 0;
  bool capture(const cgpui::PlatformWindow& target) {
    hwnd = ordinary_hwnd(target);
    style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    extended = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    const auto metrics = target.frame_metrics();
    return metrics.supported && metrics.client_title_bar && GetMenu(hwnd) == nullptr &&
        (style & WS_CAPTION) == WS_CAPTION && !(style & (WS_CHILD | WS_POPUP)) &&
        !(extended & (WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP)) &&
        GetWindowRect(hwnd, &window) && GetClientRect(hwnd, &client) && ClientToScreen(hwnd, &origin) &&
        origin.y == window.top && client.right > 64 && client.bottom > 64;
  }
  bool same(const cgpui::PlatformWindow& target) const {
    ClientAlphaTopology other;
    return other.capture(target) && hwnd == other.hwnd && style == other.style && extended == other.extended &&
        EqualRect(&window, &other.window) && EqualRect(&client, &other.client) &&
        origin.x == other.origin.x && origin.y == other.origin.y;
  }
};
inline void client_alpha_regions(cgpui::PlatformWindow& window) {
  const auto state = window.state();
  const float width = state.framebuffer_size.width / state.scale.value;
  const float y = window.frame_metrics().client_resize_border.top + 8;
  using cgpui::WindowControlArea;
  window.set_window_control_regions({
      {WindowControlArea::caption, {{0, 0}, {width, y + 12}}},
      {WindowControlArea::client, {{width / 2 - 8, y - 6}, {16, 12}}},
      {WindowControlArea::maximize_button, {{width * 5 / 6 - 8, y - 6}, {16, 12}}},
  });
}
inline bool client_alpha_hits(const cgpui::PlatformWindow& window) {
  ClientAlphaTopology native;
  if (!native.capture(window)) return false;
  const auto state = window.state();
  const LONG y = LONG(std::lround((window.frame_metrics().client_resize_border.top + 8) * state.scale.value));
  const auto hit = [&](POINT point, LRESULT expected) {
    return SendMessageW(native.hwnd, WM_NCHITTEST, 0, MAKELPARAM(point.x, point.y)) == expected;
  };
  return hit({native.origin.x + native.client.right / 6, native.origin.y + y}, HTCAPTION) &&
      hit({native.origin.x + native.client.right / 2, native.origin.y + y}, HTCLIENT) &&
      hit({native.origin.x + native.client.right * 5 / 6, native.origin.y + y}, HTMAXBUTTON) &&
      hit({native.window.left + native.client.right / 2, native.window.top + 1}, HTTOP) &&
      hit({native.window.left + 1, native.window.top + native.client.bottom / 2}, HTLEFT);
}
inline bool client_alpha_paint(cgpui::Renderer& renderer, const cgpui::PlatformWindow& window,
                                bool blue, bool desktop = true) {
  if (!ordinary_draw_bands(renderer, window, blue)) return false;
  const auto* pixels = renderer.last_frame_pixels();
  const unsigned y = unsigned(std::lround((window.frame_metrics().client_resize_border.top + 8) *
                                          window.state().scale.value));
  const auto half = blue ? std::array<int, 4>{0, 0, 128, 128} : std::array<int, 4>{128, 0, 0, 128};
  const auto full = blue ? std::array<int, 4>{0, 0, 255, 255} : std::array<int, 4>{255, 0, 0, 255};
  if (!pixels || y >= pixels->height || !expect_texture_pixel(*pixels, pixels->width / 6, y, half) ||
      !expect_texture_pixel(*pixels, pixels->width / 2, y, {0, 0, 0, 0}) ||
      !expect_texture_pixel(*pixels, pixels->width * 5 / 6, y, full)) return false;
  if (!desktop) return true; // A real material may replace the controlled green desktop backing.
  POINT origin{};
  return ClientToScreen(ordinary_hwnd(window), &origin) && ordinary_desktop_bands(window, blue, true) &&
      expect_screen_color(blue ? ScreenColor{0, 127, 128} : ScreenColor{128, 127, 0},
                          {origin.x + LONG(pixels->width / 6), origin.y + LONG(y)}) &&
      expect_screen_color({0, 255, 0}, {origin.x + LONG(pixels->width / 2), origin.y + LONG(y)}) &&
      expect_screen_color(blue ? ScreenColor{0, 0, 255} : ScreenColor{255, 0, 0},
                          {origin.x + LONG(pixels->width * 5 / 6), origin.y + LONG(y)});
}
inline bool client_alpha_scale_pixels(cgpui::Renderer& renderer, const cgpui::PlatformWindow& window,
                                       bool doubled) {
  const auto size = window.state().framebuffer_size;
  if (!renderer.resize(size, {doubled ? 2.0F : 1.0F})) return false;
  auto frame = renderer.begin_frame();
  if (!frame) return false;
  (*frame)->clear({1, 1, 1, 0});
  (*frame)->draw_rect({.rect = {{4, 4}, {8, 8}}, .color = {1, 0, 0, 0.5F}});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present()) return false;
  const auto* pixels = renderer.last_frame_pixels();
  const unsigned inside = doubled ? 20 : 6;
  POINT origin{};
  return pixels && ClientToScreen(ordinary_hwnd(window), &origin) &&
      expect_texture_pixel(*pixels, inside, inside, {128, 0, 0, 128}) &&
      expect_texture_pixel(*pixels, 28, 12, {0, 0, 0, 0}) &&
      expect_screen_color({128, 127, 0}, {origin.x + LONG(inside), origin.y + LONG(inside)}) &&
      expect_screen_color({0, 255, 0}, {origin.x + 28, origin.y + 12}) &&
      (doubled || (expect_texture_pixel(*pixels, 20, 20, {0, 0, 0, 0}) &&
                   expect_screen_color({0, 255, 0}, {origin.x + 20, origin.y + 20})));
}
} // namespace cgpui_test
