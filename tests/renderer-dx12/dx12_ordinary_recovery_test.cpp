#include "../support/frame_wait_test_support.hpp"
#include "dx12_ordinary_pixels_support.hpp"
#include "dx12_ordinary_menu_loop_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool paint_peer(Renderer& renderer) {
  auto frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) return false;
  (*frame)->clear({0, 1, 0, 1});
  return (*frame)->request_pixel_capture() && (*frame)->present() &&
      dx12_pixel(renderer.last_frame_pixels(), 8, 8, {0, 255, 0, 255});
}
bool menu_matches(const MenuLoopObservation& expected, const MenuLoopObservation& actual) {
  const auto before = expected.after.different(actual.before);
  const auto opened = expected.opened.different(actual.opened);
  const auto after = expected.after.different(actual.after);
  const bool focus = expected.after_focus == actual.before_focus &&
      expected.opened_focus == actual.opened_focus && expected.after_focus == actual.after_focus;
  std::cout << "RECOVERED_NATIVE_MENU before=" << before << " opened=" << opened
            << " after=" << after << " focus_equal=" << focus << std::endl;
  return actual.ok && !before && !opened && !after && focus;
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  OrdinaryDesktop desktop;
  if (!deadline.done || !desktop.valid()) return dx12_fail(1, "ordinary recovery desktop unavailable");
  auto app = create_platform_application();
  if (!app) return dx12_fail(2, app.error().message.c_str());
  MenuCommands commands;
  auto window = (*app)->create_window(desktop.descriptor(true),
      [&](const PlatformEvent& event) { commands.observe(event); });
  if (!window || !ordinary_install_menu(**app, **window) || !desktop.place(**window))
    return dx12_fail(3, "ordinary recovery native-menu fixture differs");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  const HWND hwnd = ordinary_hwnd(**window);
  const HMENU menu = GetMenu(hwnd);
  const auto reference = native_menu_loop(hwnd, commands, "opaque-reference");
  if (!reference.ok) return dx12_fail(4, "recovery opaque menu reference failed");
  auto renderer = ordinary_renderer(**window, true);
  Dx12Window peer_window;
  auto peer = create_dx12_renderer(peer_window.descriptor());
  if (!renderer || !peer || !paint_peer(**peer) ||
      !ordinary_draw_bands(**renderer, **window, false) ||
      !ordinary_desktop_bands(**window, false, true))
    return dx12_fail(5, "ordinary/opaque recovery cohort warmup failed");
  std::vector<std::unique_ptr<RenderFrame>> retained;
  for (unsigned epoch = 0; epoch != 2; ++epoch) {
    const auto before = (*renderer)->backend_info(), peer_before = (*peer)->backend_info();
    if (!before.adapter_luid || before.adapter_luid != peer_before.adapter_luid)
      return dx12_fail(6, "ordinary alpha and opaque peer selected different adapters");
    auto pending = cgpui_test::wait_for_test_frame(**renderer);
    if (!pending) return dx12_fail(7, "ordinary pending pre-removal frame unavailable");
    (*pending)->clear({1, 0, 1, 0.5F});
    const auto* borrowed = (*renderer)->last_frame_pixels();
    if (!borrowed) return dx12_fail(8, "ordinary pre-removal captured pixels unavailable");
    const auto saved = borrowed->rgba8;
    if (!dx12_remove_native_device(before)) return dx12_fail(9, "ordinary native device removal failed");
    const auto peer_retry = (*peer)->begin_frame();
    if (peer_retry || peer_retry.error().code != ErrorCode::frame_retry_required ||
        (*peer)->backend_info().device_generation != peer_before.device_generation + 1 ||
        (*renderer)->backend_info().device_generation != before.device_generation ||
        (*renderer)->last_frame_pixels() || (*peer)->last_frame_pixels() || borrowed->rgba8 != saved)
      return dx12_fail(10, "ordinary alpha host blocked opaque peer recovery or invalidated CPU capture");
    const auto retry = (*pending)->present();
    if (retry || retry.error().code != ErrorCode::frame_retry_required)
      return dx12_fail(11, "ordinary pending frame did not rebuild and request retry");
    const auto after = (*renderer)->backend_info();
    if (after.device_generation != before.device_generation + 1 ||
        after.adapter_luid != before.adapter_luid || after.software_adapter != before.software_adapter ||
        (*renderer)->last_frame_pixels())
      return dx12_fail(12, "ordinary recovery generation, pinned adapter or capture differs");
    const auto repeated = (*pending)->present();
    if (repeated || repeated.error().code != ErrorCode::invalid_argument)
      return dx12_fail(13, "old ordinary CPU frame presented twice");
    retained.push_back(std::move(*pending));
    const bool blue = epoch == 0; // Red -> blue -> red rules out a stale old visual.
    if (!paint_peer(**peer) || !ordinary_draw_bands(**renderer, **window, blue) ||
        !ordinary_desktop_bands(**window, blue, true) ||
        !ordinary_composition((*renderer)->surface_composition(), true))
      return dx12_fail(14, "recovered ordinary/opaque cohort pixels or alpha differ");
    const auto observed = native_menu_loop(hwnd, commands, blue ? "recovered-blue" : "recovered-red");
    if (!menu_matches(reference, observed) || ordinary_hwnd(**window) != hwnd || GetMenu(hwnd) != menu)
      return dx12_fail(15, "recovered native menu differs from the same opaque reference");
    if (!dx12_recovered_native_device_clean(after)) return dx12_fail(16, "recovered native debug errors");
  }
  auto active = cgpui_test::wait_for_test_frame(**renderer);
  if (!active) return dx12_fail(17, "post-recovery active frame unavailable");
  retained.clear();
  const auto overlap = (*renderer)->begin_frame();
  if (overlap || overlap.error().code != ErrorCode::invalid_argument)
    return dx12_fail(18, "old ordinary frame destruction unlocked a new active frame");
  active->reset();
  return ordinary_draw_bands(**renderer, **window, true) && ordinary_desktop_bands(**window, true, true)
      ? 0 : dx12_fail(19, "ordinary rendering failed after old-generation frame destruction");
}
