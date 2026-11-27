#include "../support/frame_wait_test_support.hpp"
#include "dx12_popup_test_support.hpp"
#include "cgpui/platform/platform.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
Win32SurfaceHandle surface(const PlatformWindow& window) {
  return std::get<Win32SurfaceHandle>(window.native_surface());
}
HWND hwnd(const PlatformWindow& window) { return static_cast<HWND>(surface(window).hwnd); }
Result<std::unique_ptr<Renderer>> renderer_for(const PlatformWindow& window, bool transparent) {
  const auto state = window.state();
  return create_dx12_renderer({.native_surface = window.native_surface(),
      .framebuffer_size = state.framebuffer_size, .scale = state.scale,
      .transparent_background = transparent});
}

int check_popup(PlatformApplication& app, PlatformWindow& parent, bool pointer_transparent) {
  const WindowPopupPlacement placement{.anchor = {{16, 16}, {0, 0}}};
  WindowDescriptor descriptor{.size = {64, 64}, .kind = WindowKind::popup, .popup = placement};
  descriptor.popup_options.pointer_transparent = pointer_transparent;
  const HWND active_before = GetActiveWindow();
  auto popup = app.create_child_window(descriptor, parent, [](const PlatformEvent&) {});
  if (!popup) return dx12_fail(20, popup.error().message.c_str());
  pump_messages_for(50);
  const HWND popup_hwnd = hwnd(**popup), parent_hwnd = hwnd(parent);
  const LONG_PTR style = GetWindowLongPtrW(popup_hwnd, GWL_STYLE);
  const LONG_PTR extended = GetWindowLongPtrW(popup_hwnd, GWL_EXSTYLE);
  constexpr LONG_PTR required = WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_NOREDIRECTIONBITMAP;
  constexpr LONG_PTR transparent_style = WS_EX_LAYERED | WS_EX_TRANSPARENT;
  if (!surface(**popup).composition || !(style & WS_POPUP) ||
      (style & (WS_CAPTION | WS_THICKFRAME | WS_CHILD)) ||
      (extended & required) != required ||
      (extended & WS_EX_APPWINDOW) ||
      (extended & transparent_style) != (pointer_transparent ? transparent_style : 0) ||
      GetWindow(popup_hwnd, GW_OWNER) != parent_hwnd || !IsWindowVisible(popup_hwnd) ||
      IsIconic(popup_hwnd) || GetActiveWindow() != active_before ||
      GetForegroundWindow() == popup_hwnd)
    return dx12_fail(21, "popup surface, ownership, input style or nonactivation differs");
  const auto placed = (*popup)->place_popup(placement);
  const auto state = (*popup)->state();
  POINT parent_origin{}, popup_origin{};
  RECT bounds{};
  if (!ClientToScreen(parent_hwnd, &parent_origin) || !ClientToScreen(popup_hwnd, &popup_origin) ||
      !GetWindowRect(popup_hwnd, &bounds)) return dx12_fail(22, "popup screen coordinates unavailable");
  const float scale = parent.state().scale.value;
  if (!placed.supported || !placed.placed || placed.flipped || placed.shifted ||
      std::abs(placed.bounds.origin.x - 16) > 1 || std::abs(placed.bounds.origin.y - 16) > 1 ||
      std::abs(bounds.left - parent_origin.x - std::lround(16 * scale)) > 1 ||
      std::abs(bounds.top - parent_origin.y - std::lround(16 * scale)) > 1 ||
      std::abs(state.framebuffer_size.width - std::lround(64 * state.scale.value)) > 1 ||
      std::abs(state.framebuffer_size.height - std::lround(64 * state.scale.value)) > 1)
    return dx12_fail(23, "popup placement or DPI-scaled extent differs");
  auto renderer = renderer_for(**popup, true);
  if (!renderer) return dx12_fail(24, renderer.error().message.c_str());
  if (!(*renderer)->popup_windows_supported() || !(*renderer)->surface_composition().alpha_composited())
    return dx12_fail(25, "popup renderer did not expose alpha presentation");
  const float width = state.framebuffer_size.width / state.scale.value;
  const float height = state.framebuffer_size.height / state.scale.value;
  for (const bool blue : {false, true}) {
    auto frame = cgpui_test::wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(26, "platform popup begin failed");
    (*frame)->clear({0, 0, 0, 0});
    (*frame)->draw_rect({.rect = {{0, 0}, {width / 3, height}},
        .color = blue ? Color{0, 0, 1, 0.5F} : Color{1, 0, 0, 0.5F}});
    (*frame)->draw_rect({.rect = {{width * 2 / 3, 0}, {width / 3, height}},
        .color = blue ? Color{0, 0, 1, 1} : Color{1, 0, 0, 1}});
    if (!(*frame)->request_pixel_capture() || !(*frame)->present())
      return dx12_fail(27, "platform popup capture/present failed");
    const auto* pixels = (*renderer)->last_frame_pixels();
    if (!pixels || !pixels->valid() || pixels->width != unsigned(state.framebuffer_size.width) ||
        pixels->height != unsigned(state.framebuffer_size.height)) return dx12_fail(28, "popup capture extent differs");
    const auto half = blue ? std::array<int, 4>{0, 0, 128, 128} : std::array<int, 4>{128, 0, 0, 128};
    const auto full = blue ? std::array<int, 4>{0, 0, 255, 255} : std::array<int, 4>{255, 0, 0, 255};
    const unsigned x = pixels->width / 6, y = pixels->height / 2, solid_x = pixels->width * 5 / 6;
    const POINT half_point{popup_origin.x + LONG(x), popup_origin.y + LONG(y)};
    const POINT clear_point{popup_origin.x + LONG(pixels->width / 2), half_point.y};
    const POINT solid_point{popup_origin.x + LONG(solid_x), half_point.y};
    if (!expect_texture_pixel(*pixels, x, y, half) ||
        !expect_texture_pixel(*pixels, pixels->width / 2, y, {0, 0, 0, 0}) ||
        !expect_texture_pixel(*pixels, solid_x, y, full) ||
        !expect_screen_color(blue ? ScreenColor{0, 127, 128} : ScreenColor{128, 127, 0}, half_point) ||
        !expect_screen_color({0, 255, 0}, clear_point) ||
        !expect_screen_color(blue ? ScreenColor{0, 0, 255} : ScreenColor{255, 0, 0}, solid_point))
      return dx12_fail(29, "platform popup texture/desktop alpha differs");
    if (WindowFromPoint(solid_point) != (pointer_transparent ? parent_hwnd : popup_hwnd) ||
        GetActiveWindow() != active_before || GetForegroundWindow() == popup_hwnd)
      return dx12_fail(30, "popup input pass-through or nonactivation differs after presentation");
  }
  return 0;
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  if (!deadline.done || !dx12_popup_desktop_ready()) return dx12_fail(1, "platform popup desktop unavailable");
  auto app = create_platform_application();
  if (!app || !(*app)->popup_windows_supported()) return dx12_fail(2, "platform popup support unavailable");
  RECT work{};
  if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0)) return dx12_fail(3, "work area unavailable");
  auto parent = (*app)->create_window({.title = "DX12 platform popup acceptance", .size = {176, 144},
      .position = Point{float(work.left + 16), float(work.top + 16)},
      .chrome = {.titlebar_visible = false, .decorations = false, .resizable = false}},
      [](const PlatformEvent&) {});
  if (!parent) return dx12_fail(4, parent.error().message.c_str());
  if (surface(**parent).composition ||
      !SetWindowPos(hwnd(**parent), HWND_TOPMOST, work.left + 16, work.top + 16, 0, 0,
                    SWP_NOSIZE | SWP_NOACTIVATE)) return dx12_fail(5, "opaque parent fixture differs");
  pump_messages_for(50);
  RECT bounds{};
  if (!GetWindowRect(hwnd(**parent), &bounds) || bounds.left < work.left || bounds.top < work.top ||
      bounds.right > work.right || bounds.bottom > work.bottom)
    return dx12_fail(6, "work area cannot contain the fixture; desktop alpha was not tested");
  auto renderer = renderer_for(**parent, false);
  if (!renderer) return dx12_fail(7, renderer.error().message.c_str());
  if (!(*renderer)->popup_windows_supported() || (*renderer)->surface_composition().alpha_composited())
    return dx12_fail(8, "opaque DX12 parent did not advertise popup support");
  auto frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!frame) return dx12_fail(9, "parent begin failed");
  (*frame)->clear({0, 1, 0, 1});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 8, 8, {0, 255, 0, 255}))
    return dx12_fail(10, "parent green pixels unavailable");
  POINT control{8, 8};
  if (!ClientToScreen(hwnd(**parent), &control) || !expect_screen_color({0, 255, 0}, control))
    return dx12_fail(11, "real desktop parent control pixel differs");
  for (const bool pointer_transparent : {false, true}) {
    std::cout << "DX12 platform popup pointer-transparent=" << pointer_transparent << '\n';
    if (const int result = check_popup(**app, **parent, pointer_transparent)) return result;
    pump_messages_for(50); // Retire the previous popup before hit-testing the next branch.
  }
  return 0;
}
