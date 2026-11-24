#include "dx12_dynamic_alpha_test_support.hpp"
#include "dx12_ordinary_pixels_support.hpp"
#include "dx12_ordinary_menu_pixels_support.hpp"
#include <cmath>

namespace cgpui_test {
int dynamic_alpha_opaque_case() {
  using namespace cgpui;
  Dx12RecoveryDeadline deadline;
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return 1;
  auto window = (*app)->create_window(desktop.descriptor(false), [](const PlatformEvent&) {});
  if (!window || !ordinary_install_menu(**app, **window) || !desktop.place(**window)) return 2;
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  const HWND hwnd = ordinary_hwnd(**window);
  MenuPixels reference; MenuFocus focus;
  if (!stable_menu(hwnd, reference, focus)) return 3;
  auto renderer = ordinary_renderer(**window, false);
  if (!renderer || !ordinary_composition((*renderer)->surface_composition(), false)) return 4;
  const auto size = (*window)->state();
  const float width = size.framebuffer_size.width / size.scale.value;
  const float height = size.framebuffer_size.height / size.scale.value;
  unsigned iteration = 0;
  for (const float alpha : {0.0F, .5F, 1.0F}) {
    const bool blue = (++iteration % 2) == 0; // Distinct desktop color detects stale first presentation.
    auto frame = wait_for_test_frame(**renderer);
    if (!frame) return 5;
    (*frame)->clear(blue ? Color{0, 0, 1, alpha} : Color{1, 0, 0, alpha});
    (*frame)->draw_rect({.rect = {{width / 3, 0}, {width / 3, height}}, .color = {0, 1, 0, .5F}});
    (*frame)->draw_rect({.rect = {{width * 2 / 3, 0}, {width / 3, height}}, .color = {1, 1, 1, 1}});
    if (!(*frame)->request_pixel_capture() || !(*frame)->present()) return 6;
    const auto* pixels = (*renderer)->last_frame_pixels();
    const int a = int(std::lround(alpha * 255)), mixed_a = int(std::lround((.5F + .5F * alpha) * 255));
    if (!pixels || !expect_texture_pixel(*pixels, pixels->width / 6, pixels->height / 2,
          blue ? std::array<int, 4>{0, 0, 255, a} : std::array<int, 4>{255, 0, 0, a}) ||
        !expect_texture_pixel(*pixels, pixels->width / 2, pixels->height / 2,
          blue ? std::array<int, 4>{0, 128, 128, mixed_a} : std::array<int, 4>{128, 128, 0, mixed_a}) ||
        !expect_texture_pixel(*pixels, pixels->width * 5 / 6, pixels->height / 2, {255, 255, 255, 255})) return 7;
    RECT client{}; POINT origin{};
    if (!GetClientRect(hwnd, &client) || !ClientToScreen(hwnd, &origin)) return 8;
    const LONG y = origin.y + client.bottom / 2;
    if (!expect_screen_color(blue ? ScreenColor{0, 0, 255} : ScreenColor{255, 0, 0}, {origin.x + client.right / 6, y}) ||
        !expect_screen_color(blue ? ScreenColor{0, 128, 128} : ScreenColor{128, 128, 0}, {origin.x + client.right / 2, y}) ||
        !expect_screen_color({255, 255, 255}, {origin.x + client.right * 5 / 6, y})) return 9;
    MenuPixels actual; MenuFocus actual_focus;
    if (!stable_menu(hwnd, actual, actual_focus) || reference.different(actual) || focus != actual_focus) return 10;
  }
  return 0;
}
} // namespace cgpui_test
