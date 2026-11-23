#pragma once

#include "dx12_client_alpha_test_support.hpp"

namespace cgpui_test {
inline float controls_row(const cgpui::PlatformWindow& window) {
  return window.frame_metrics().client_resize_border.top + 8;
}
inline void controls_regions(cgpui::PlatformWindow& window) {
  const auto state = window.state();
  const float width = state.framebuffer_size.width / state.scale.value;
  const float row = controls_row(window);
  using cgpui::WindowControlArea;
  window.set_window_control_regions({
      {WindowControlArea::caption, {{0, 0}, {width, row + 14}}},
      {WindowControlArea::client, {{width / 2 - 8, 0}, {16, row + 14}}},
      {WindowControlArea::minimize_button, {{width * 2 / 3 - 8, row - 6}, {16, 12}}},
      {WindowControlArea::maximize_button, {{width * 5 / 6 - 8, row - 6}, {16, 12}}},
      {WindowControlArea::close_button, {{width - 24, row - 6}, {16, 12}}},
  });
}
inline bool control_button(cgpui::PlatformWindow& window, WPARAM expected) {
  controls_regions(window);
  const HWND hwnd = ordinary_hwnd(window);
  const auto state = window.state();
  const float width = state.framebuffer_size.width / state.scale.value;
  const float x = expected == HTCLOSE ? width - 16 : width * 5 / 6;
  POINT point{LONG(std::lround(x * state.scale.value)),
              LONG(std::lround(controls_row(window) * state.scale.value))};
  if (!ClientToScreen(hwnd, &point)) return false;
  const LPARAM at = MAKELPARAM(point.x, point.y);
  if (SendMessageW(hwnd, WM_NCHITTEST, 0, at) != LRESULT(expected)) return false;
  SendMessageW(hwnd, WM_NCLBUTTONDOWN, expected, at);
  SendMessageW(hwnd, WM_NCLBUTTONUP, expected, at);
  return true;
}
inline bool controls_wait(const auto& ready) {
  const ULONGLONG end = GetTickCount64() + 3000;
  do {
    pump_messages_for(20);
    if (ready()) return true;
  } while (GetTickCount64() < end);
  return false;
}
inline bool controls_paint(cgpui::Renderer& renderer, cgpui::PlatformWindow& window,
                          bool blue, bool desktop = true) {
  const auto state = window.state();
  controls_regions(window);
  return renderer.resize(state.framebuffer_size, state.scale).has_value() &&
      ordinary_composition(renderer.surface_composition(), true) &&
      client_alpha_paint(renderer, window, blue, desktop);
}
inline bool fixed_top_hits(const cgpui::PlatformWindow& window) {
  RECT bounds{}, client{};
  POINT origin{};
  const HWND hwnd = ordinary_hwnd(window);
  if (!GetWindowRect(hwnd, &bounds) || !GetClientRect(hwnd, &client) ||
      !ClientToScreen(hwnd, &origin) || origin.y != bounds.top) return false;
  const auto hit = [&](LONG x) {
    return SendMessageW(hwnd, WM_NCHITTEST, 0, MAKELPARAM(origin.x + x, bounds.top + 1));
  };
  return hit(client.right / 6) == HTCAPTION && hit(client.right / 2) == HTCLIENT &&
      window.frame_metrics().client_resize_border.top == 0 &&
      !(GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_THICKFRAME);
}
} // namespace cgpui_test
