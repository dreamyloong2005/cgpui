#include "dx12_popup_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool composition_matches(const RendererSurfaceComposition& composition, bool transparent) {
  return composition.supported && composition.transparent_requested == transparent &&
         composition.composite_alpha == RendererCompositeAlpha::pre_multiplied;
}

bool clear_and_check(Renderer& renderer, const CompositionTestWindows& windows, Color clear,
                     std::array<int, 4> pixel, ScreenColor screen) {
  auto frame = renderer.begin_frame();
  if (!frame) return false;
  (*frame)->clear(clear);
  if (!(*frame)->request_pixel_capture() || !(*frame)->present()) return false;
  const auto* pixels = renderer.last_frame_pixels();
  return pixels && pixels->width == 64 && pixels->height == 64 &&
      expect_texture_pixel(*pixels, 32, 32, pixel) &&
      windows.expect({0, 255, 0}, windows.backdrop_point()) &&
      windows.expect(screen, windows.popup_point(32, 32));
}

int check_ordinary_refusal() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(20, "ordinary HWND unavailable");
  auto descriptor = window.descriptor();
  descriptor.transparent_background = true;
  auto rejected = create_dx12_renderer(descriptor);
  if (rejected || rejected.error().code != ErrorCode::unsupported_platform)
    return dx12_fail(21, "ordinary HWND accepted transparent creation");
  descriptor.transparent_background = false;
  auto renderer = create_dx12_renderer(descriptor);
  if (!renderer) return dx12_fail(22, renderer.error().message.c_str());
  const auto refused = (*renderer)->set_transparent_surface(true);
  if (refused || refused.error().code != ErrorCode::unsupported_platform)
    return dx12_fail(23, "ordinary HWND accepted transparent intent");
  const auto unchanged = (*renderer)->surface_composition();
  const auto opaque = (*renderer)->set_transparent_surface(false);
  if (!unchanged.supported || unchanged.transparent_requested ||
      unchanged.composite_alpha != RendererCompositeAlpha::opaque || !opaque ||
      !opaque->supported || opaque->transparent_requested ||
      opaque->composite_alpha != RendererCompositeAlpha::opaque)
    return dx12_fail(24, "ordinary HWND transparency refusal changed its opaque contract");
  return 0;
}

int check_invalid_hosts(HWND popup) {
  Dx12Window ordinary;
  if (!ordinary.hwnd) return dx12_fail(30, "invalid-host ordinary fixture unavailable");
  auto descriptor = ordinary.descriptor();
  std::get<Win32SurfaceHandle>(descriptor.native_surface).composition = true;
  auto rejected = create_dx12_renderer(descriptor);
  if (rejected || rejected.error().code != ErrorCode::invalid_argument)
    return dx12_fail(31, "ordinary HWND mislabeled composition was not invalid_argument");
  auto valid = create_dx12_renderer(ordinary.descriptor());
  if (!valid) return dx12_fail(32, "mislabeled request poisoned the same ordinary HWND");

  descriptor = dx12_popup_descriptor(popup);
  std::get<Win32SurfaceHandle>(descriptor.native_surface).composition = false;
  descriptor.transparent_background = false; // Isolate topology, not alpha intent.
  rejected = create_dx12_renderer(descriptor);
  if (rejected || rejected.error().code != ErrorCode::invalid_argument)
    return dx12_fail(33, "NOREDIRECTION HWND mislabeled ordinary was not invalid_argument");
  // main next creates and presents a valid renderer on this exact popup HWND;
  // an invalid request must not occupy its DComp target or alter the window.
  Dx12Window destroyed;
  if (!destroyed.hwnd) return dx12_fail(34, "destroyed-HWND fixture unavailable");
  descriptor = destroyed.descriptor();
  if (!DestroyWindow(destroyed.hwnd)) return dx12_fail(35, "destroyed-HWND fixture failed");
  destroyed.hwnd = nullptr;
  rejected = create_dx12_renderer(descriptor); // No intervening HWND allocation/reuse.
  if (rejected || rejected.error().code != ErrorCode::invalid_argument)
    return dx12_fail(36, "destroyed HWND was not invalid_argument");
  return 0;
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done || !dx12_popup_desktop_ready())
    return dx12_fail(1, "popup surface desktop/DPI/watchdog unavailable");
  CompositionTestWindows windows(48, 48);
  if (!windows.valid() || !IsWindowVisible(windows.popup()) || IsIconic(windows.popup()))
    return dx12_fail(2, "visible popup surface fixture unavailable");
  if (const int result = check_invalid_hosts(windows.popup())) return result;
  auto descriptor = dx12_popup_descriptor(windows.popup());
  descriptor.transparent_background = false;
  auto renderer = create_dx12_renderer(descriptor);
  if (!renderer) return dx12_fail(3, renderer.error().message.c_str());
  if (!composition_matches((*renderer)->surface_composition(), false))
    return dx12_fail(4, "composition popup with opaque intent lost premultiplied presentation");
  if (!clear_and_check(**renderer, windows, {1, 0, 0, 0.5F}, {128, 0, 0, 128}, {128, 127, 0}))
    return dx12_fail(5, "nonblack fractional clear was not premultiplied on capture and desktop");
  const auto generation = (*renderer)->backend_info().device_generation;
  for (const bool transparent : {true, false}) {
    const auto changed = (*renderer)->set_transparent_surface(transparent);
    if (!changed || !composition_matches(*changed, transparent) ||
        !composition_matches((*renderer)->surface_composition(), transparent) ||
        (*renderer)->backend_info().device_generation != generation)
      return dx12_fail(6, "popup intent switch changed fixed composition or device generation");
    if (transparent) {
      if (!clear_and_check(**renderer, windows, {1, 0, 1, 0}, {0, 0, 0, 0}, {0, 255, 0}))
        return dx12_fail(7, "nonblack alpha-zero clear leaked RGB into desktop composition");
    } else if (!clear_and_check(**renderer, windows, {0, 0, 1, 0.5F}, {0, 0, 128, 128}, {0, 127, 128})) {
      return dx12_fail(8, "opaque popup intent stopped actual fractional-alpha presentation");
    }
  }
  if (!SetWindowPos(windows.popup(), nullptr, 0, 0, 80, 48,
                    SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE))
    return dx12_fail(9, "actual popup HWND resize failed");
  pump_messages_for(50);
  RECT client{};
  if (!GetClientRect(windows.popup(), &client) || client.right != 80 || client.bottom != 48)
    return dx12_fail(10, "actual popup client extent did not resize");
  // Exercise renderer DPI updates; this is not a physical monitor migration.
  for (const bool doubled : {true, false}) {
    if (!(*renderer)->resize({80, 48}, {doubled ? 2.0F : 1.0F}))
      return dx12_fail(11, "popup resize/DPI update failed");
    auto frame = (*renderer)->begin_frame();
    if (!frame) return dx12_fail(12, "resized popup begin failed");
    (*frame)->clear({1, 1, 1, 0});
    (*frame)->draw_rect({.rect = {{4, 4}, {8, 8}}, .color = {1, 0, 0, 0.5F}});
    if (!(*frame)->request_pixel_capture() || !(*frame)->present())
      return dx12_fail(13, "resized popup capture/present failed");
    const auto* pixels = (*renderer)->last_frame_pixels();
    const unsigned inside = doubled ? 20U : 6U;
    if (!pixels || pixels->width != 80 || pixels->height != 48 ||
        !expect_texture_pixel(*pixels, inside, inside, {128, 0, 0, 128}) ||
        !expect_texture_pixel(*pixels, 28, 12, {0, 0, 0, 0}) ||
        (!doubled && !expect_texture_pixel(*pixels, 20, 20, {0, 0, 0, 0})) ||
        !windows.expect({128, 127, 0}, windows.popup_point(inside, inside)) ||
        !windows.expect({0, 255, 0}, windows.popup_point(28, 12)) ||
        (!doubled && !windows.expect({0, 255, 0}, windows.popup_point(20, 20))) ||
        !composition_matches((*renderer)->surface_composition(), false))
      return dx12_fail(14, "popup resized/DPI-scaled pixels or continued presentation differ");
  }
  return check_ordinary_refusal();
}
