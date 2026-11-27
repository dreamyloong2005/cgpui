#include "../support/frame_wait_test_support.hpp"
#include "dx12_ordinary_pixels_support.hpp"
#include "dx12_ordinary_menu_loop_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool resize_window(PlatformWindow& window, bool smaller) {
  if (!SetWindowPos(ordinary_hwnd(window), nullptr, 0, 0, smaller ? 376 : 400,
                    smaller ? 248 : 280, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE)) return false;
  pump_messages_for(100);
  RECT client{};
  const auto state = window.state();
  return GetClientRect(ordinary_hwnd(window), &client) && client.right == state.framebuffer_size.width &&
      client.bottom == state.framebuffer_size.height;
}
bool fixed_alpha(const RendererSurfaceComposition& report, bool intent) {
  return report.supported && report.transparent_requested == intent &&
      report.composite_alpha == RendererCompositeAlpha::pre_multiplied;
}
bool dpi_pixels(Renderer& renderer, const PlatformWindow& window, bool doubled) {
  const auto size = window.state().framebuffer_size;
  if (!renderer.resize(size, {doubled ? 2.0F : 1.0F})) return false;
  auto frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) return false;
  (*frame)->clear({1, 1, 1, 0});
  (*frame)->draw_rect({.rect = {{4, 4}, {8, 8}}, .color = {1, 0, 0, 0.5F}});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present()) return false;
  const unsigned inside = doubled ? 20U : 6U;
  const auto* pixels = renderer.last_frame_pixels();
  POINT origin{};
  if (!pixels || pixels->width != unsigned(size.width) || pixels->height != unsigned(size.height) ||
      !ClientToScreen(ordinary_hwnd(window), &origin)) return false;
  return expect_texture_pixel(*pixels, inside, inside, {128, 0, 0, 128}) &&
      expect_texture_pixel(*pixels, 28, 12, {0, 0, 0, 0}) &&
      expect_screen_color({128, 127, 0}, {origin.x + LONG(inside), origin.y + LONG(inside)}) &&
      expect_screen_color({0, 255, 0}, {origin.x + 28, origin.y + 12}) &&
      (doubled || (expect_texture_pixel(*pixels, 20, 20, {0, 0, 0, 0}) &&
                   expect_screen_color({0, 255, 0}, {origin.x + 20, origin.y + 20})));
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  OrdinaryDesktop desktop;
  if (!deadline.done || !desktop.valid()) return dx12_fail(1, "ordinary resize fixture unavailable");
  auto app = create_platform_application();
  if (!app) return dx12_fail(2, app.error().message.c_str());
  MenuCommands commands;
  auto window = (*app)->create_window(desktop.descriptor(true),
      [&](const PlatformEvent& event) { commands.observe(event); });
  if (!window || !ordinary_install_menu(**app, **window) || !desktop.place(**window))
    return dx12_fail(3, "ordinary resize native-menu fixture differs");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  const HWND hwnd = ordinary_hwnd(**window);
  const HMENU menu = GetMenu(hwnd);
  const LONG_PTR extended = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
  std::array<MenuLoopObservation, 2> references;
  for (const bool smaller : {false, true}) {
    if (!resize_window(**window, smaller)) return dx12_fail(4, "opaque reference resize failed");
    references[smaller] = native_menu_loop(hwnd, commands, smaller ? "opaque-small" : "opaque-large");
    if (!references[smaller].ok) return dx12_fail(5, "opaque resized native menu reference failed");
  }
  auto renderer = ordinary_renderer(**window, true);
  if (!renderer) return dx12_fail(6, renderer.error().message.c_str());
  const auto generation = (*renderer)->backend_info().device_generation;
  for (const bool smaller : {false, true}) {
    const bool intent = !smaller;
    const auto changed = (*renderer)->set_transparent_surface(intent);
    if (!changed || !fixed_alpha(*changed, intent) || !resize_window(**window, smaller))
      return dx12_fail(7, "ordinary resize/intent change lost fixed alpha composition");
    // Explicit renderer scale changes, not a claim of physical monitor migration.
    if (!dpi_pixels(**renderer, **window, true) || !dpi_pixels(**renderer, **window, false))
      return dx12_fail(8, "ordinary resized DPI-scaled GPU/desktop pixels differ");
    const auto state = (*window)->state();
    if (!(*renderer)->resize(state.framebuffer_size, state.scale) ||
        !ordinary_draw_bands(**renderer, **window, smaller) ||
        !ordinary_desktop_bands(**window, smaller, true))
      return dx12_fail(9, "ordinary resized frame lost desktop alpha");
    const auto actual = native_menu_loop(hwnd, commands, smaller ? "alpha-small" : "alpha-large");
    const auto& expected = references[smaller];
    if (!actual.ok || expected.after.different(actual.before) ||
        expected.opened.different(actual.opened) || expected.after.different(actual.after) ||
        expected.after_focus != actual.before_focus || expected.opened_focus != actual.opened_focus ||
        expected.after_focus != actual.after_focus)
      return dx12_fail(10, "resized native menu RGB or focus differs from same-size opaque reference");
    if (!fixed_alpha((*renderer)->surface_composition(), intent) ||
        (*renderer)->backend_info().device_generation != generation || ordinary_hwnd(**window) != hwnd ||
        GetMenu(hwnd) != menu || GetWindowLongPtrW(hwnd, GWL_EXSTYLE) != extended)
      return dx12_fail(11, "resize/intent changed HWND/menu/topology/device generation");
  }
  const auto restored = (*renderer)->set_transparent_surface(true);
  return restored && fixed_alpha(*restored, true) && ordinary_desktop_bands(**window, true, true)
      ? 0 : dx12_fail(12, "restored transparent intent changed existing alpha presentation");
}
