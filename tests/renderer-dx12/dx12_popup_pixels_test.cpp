#include "../support/frame_wait_test_support.hpp"
#include "dx12_popup_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  if (!deadline.done || !dx12_popup_desktop_ready())
    return dx12_fail(1, "popup desktop/DPI/watchdog fixture unavailable; screen alpha was not tested");
  CompositionTestWindows windows(48, 48);
  if (!windows.valid() || !IsWindowVisible(windows.popup()) || IsIconic(windows.popup()))
    return dx12_fail(2, "visible composition popup unavailable");
  if (!windows.expect({0, 255, 0}, windows.backdrop_point()))
    return dx12_fail(3, "desktop backdrop control pixel is not visible");
  auto renderer = create_dx12_renderer(dx12_popup_descriptor(windows.popup()));
  if (!renderer) return dx12_fail(4, renderer.error().message.c_str());
  const auto composition = (*renderer)->surface_composition();
  if (!composition.supported || !composition.transparent_requested ||
      composition.composite_alpha != RendererCompositeAlpha::pre_multiplied)
    return dx12_fail(5, "popup did not report actual premultiplied presentation");
  for (const bool blue : {false, true}) {
    auto frame = cgpui_test::wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(6, frame.error().message.c_str());
    (*frame)->clear({0, 0, 0, 0});
    const Color translucent = blue ? Color{0, 0, 1, 0.5F} : Color{1, 0, 0, 0.5F};
    const Color opaque = blue ? Color{0, 0, 1, 1} : Color{1, 0, 0, 1};
    (*frame)->draw_rect({.rect = {{0, 0}, {20, 64}}, .color = translucent});
    (*frame)->draw_rect({.rect = {{40, 0}, {24, 64}}, .color = opaque});
    if (!(*frame)->request_pixel_capture()) return dx12_fail(7, "popup capture request failed");
    if (const auto presented = (*frame)->present(); !presented)
      return dx12_fail(8, presented.error().message.c_str());
    const auto* pixels = (*renderer)->last_frame_pixels();
    const std::array<int, 4> half = blue ? std::array<int, 4>{0, 0, 128, 128}
                                        : std::array<int, 4>{128, 0, 0, 128};
    const std::array<int, 4> full = blue ? std::array<int, 4>{0, 0, 255, 255}
                                        : std::array<int, 4>{255, 0, 0, 255};
    if (!pixels || pixels->width != 64 || pixels->height != 64 ||
        !expect_texture_pixel(*pixels, 10, 32, half) ||
        !expect_texture_pixel(*pixels, 30, 32, {0, 0, 0, 0}) ||
        !expect_texture_pixel(*pixels, 52, 32, full))
      return dx12_fail(9, "popup texture alpha/color differs");
    // Texture alpha alone is insufficient: verify DWM's actual desktop result,
    // including alpha zero, fractional alpha and opaque content in one window.
    const ScreenColor blended = blue ? ScreenColor{0, 127, 128} : ScreenColor{128, 127, 0};
    const ScreenColor solid = blue ? ScreenColor{0, 0, 255} : ScreenColor{255, 0, 0};
    if (!windows.expect({0, 255, 0}, windows.backdrop_point()) ||
        !windows.expect(blended, windows.popup_point(10, 32)) ||
        !windows.expect({0, 255, 0}, windows.popup_point(30, 32)) ||
        !windows.expect(solid, windows.popup_point(52, 32)))
      return dx12_fail(10, "popup did not composite real per-pixel alpha on the desktop");
  }
  return 0;
}
