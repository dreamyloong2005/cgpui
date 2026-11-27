#include "dx12_ordinary_pixels_support.hpp"

#include <algorithm>
#include <cmath>

using namespace cgpui;
using namespace cgpui_test;

namespace {
struct ClientGeometry {
  RECT window{}, client{};
  POINT origin{};
  LONG_PTR style = 0, extended = 0;
};
bool client_geometry(const PlatformWindow& window, ClientGeometry& geometry) {
  const HWND hwnd = ordinary_hwnd(window);
  const auto metrics = window.frame_metrics();
  const float scale = window.state().scale.value;
  const int frame = int(std::lround(metrics.resize_border * scale));
  geometry.style = GetWindowLongPtrW(hwnd, GWL_STYLE);
  geometry.extended = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
  if (!GetWindowRect(hwnd, &geometry.window) || !GetClientRect(hwnd, &geometry.client) ||
      !ClientToScreen(hwnd, &geometry.origin)) return false;
  std::cout << "INITIAL_CLIENT_GEOMETRY client=" << metrics.client_title_bar
            << " top=" << geometry.origin.y - geometry.window.top << " frame=" << frame
            << " scale=" << scale << std::endl;
  return metrics.supported && metrics.client_title_bar && metrics.client_resize_border.top > 0 &&
      metrics.caption_buttons_reserved_width == 0 && GetMenu(hwnd) == nullptr &&
      (geometry.style & WS_CAPTION) == WS_CAPTION && (geometry.style & WS_THICKFRAME) &&
      !(geometry.style & (WS_CHILD | WS_POPUP)) &&
      !(geometry.extended & (WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP)) &&
      geometry.origin.y == geometry.window.top &&
      std::abs(geometry.origin.x - geometry.window.left - frame) <= 1 &&
      std::abs(geometry.window.right - geometry.origin.x - geometry.client.right - frame) <= 1 &&
      std::abs(geometry.window.bottom - geometry.origin.y - geometry.client.bottom - frame) <= 1;
}
bool same_geometry(const ClientGeometry& before, const ClientGeometry& after) {
  return EqualRect(&before.window, &after.window) && EqualRect(&before.client, &after.client) &&
      before.origin.x == after.origin.x && before.origin.y == after.origin.y &&
      before.style == after.style && before.extended == after.extended;
}
void client_regions(PlatformWindow& window) {
  const auto state = window.state();
  const float width = state.framebuffer_size.width / state.scale.value;
  const float y = window.frame_metrics().client_resize_border.top + 8;
  window.set_window_control_regions({
      {WindowControlArea::caption, {{0, 0}, {width, y + 12}}},
      {WindowControlArea::client, {{width / 2 - 8, y - 6}, {16, 12}}},
      {WindowControlArea::maximize_button, {{width * 5 / 6 - 8, y - 6}, {16, 12}}},
  });
}
bool client_hits(const PlatformWindow& window) {
  const HWND hwnd = ordinary_hwnd(window);
  const auto state = window.state();
  const auto metrics = window.frame_metrics();
  const float width = state.framebuffer_size.width / state.scale.value;
  const float y = metrics.client_resize_border.top + 8;
  POINT origin{};
  RECT bounds{};
  if (!ClientToScreen(hwnd, &origin) || !GetWindowRect(hwnd, &bounds)) return false;
  const auto hit = [&](POINT point, LRESULT expected) {
    const auto actual = SendMessageW(hwnd, WM_NCHITTEST, 0, MAKELPARAM(point.x, point.y));
    std::cout << "CLIENT_HIT actual=" << actual << " expected=" << expected << std::endl;
    return actual == expected;
  };
  const auto point = [&](float x, float logical_y) {
    return POINT{origin.x + LONG(std::lround(x * state.scale.value)),
                 origin.y + LONG(std::lround(logical_y * state.scale.value))};
  };
  return hit(point(width / 6, y), HTCAPTION) && hit(point(width / 2, y), HTCLIENT) &&
      hit(point(width * 5 / 6, y), HTMAXBUTTON) &&
      hit({bounds.left + (bounds.right - bounds.left) / 2, bounds.top + 1}, HTTOP) &&
      hit({bounds.left + 1, bounds.top + (bounds.bottom - bounds.top) / 2}, HTLEFT) &&
      hit(point(width / 2, y + 20), HTCLIENT);
}
bool title_row_pixels(Renderer& renderer, const PlatformWindow& window, bool blue) {
  const auto state = window.state();
  const auto* pixels = renderer.last_frame_pixels();
  const unsigned y = unsigned(std::lround(
      (window.frame_metrics().client_resize_border.top + 8) * state.scale.value));
  if (!pixels || !pixels->valid() || y >= pixels->height) return false;
  const auto half = blue ? std::array<int, 4>{0, 0, 128, 128} : std::array<int, 4>{128, 0, 0, 128};
  const auto full = blue ? std::array<int, 4>{0, 0, 255, 255} : std::array<int, 4>{255, 0, 0, 255};
  POINT origin{};
  if (!ClientToScreen(ordinary_hwnd(window), &origin)) return false;
  std::cout << "SELF_DRAWN_TITLE_ROW y=" << y << " blue=" << blue << std::endl;
  return expect_texture_pixel(*pixels, pixels->width / 6, y, half) &&
      expect_texture_pixel(*pixels, pixels->width / 2, y, {0, 0, 0, 0}) &&
      expect_texture_pixel(*pixels, pixels->width * 5 / 6, y, full) &&
      expect_screen_color(blue ? ScreenColor{0, 127, 128} : ScreenColor{128, 127, 0},
                          {origin.x + LONG(pixels->width / 6), origin.y + LONG(y)}) &&
      expect_screen_color({0, 255, 0}, {origin.x + LONG(pixels->width / 2), origin.y + LONG(y)}) &&
      expect_screen_color(blue ? ScreenColor{0, 0, 255} : ScreenColor{255, 0, 0},
                          {origin.x + LONG(pixels->width * 5 / 6), origin.y + LONG(y)});
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  OrdinaryDesktop desktop;
  if (!deadline.done || !desktop.valid()) return dx12_fail(1, "client-alpha desktop unavailable");
  auto app = create_platform_application();
  if (!app) return dx12_fail(2, app.error().message.c_str());
  auto descriptor = desktop.descriptor(true);
  descriptor.chrome.title_bar = WindowTitleBarMode::client;
  auto window = (*app)->create_window(descriptor, [](const PlatformEvent&) {});
  if (!window || !desktop.place(**window)) return dx12_fail(3, "initial client window unavailable");
  const HWND hwnd = ordinary_hwnd(**window);
  ClientGeometry initial;
  // Establish client mode BEFORE any apply_window_chrome call or renderer:
  // a later setter must not accidentally disguise an initially wrong topology.
  if (!client_geometry(**window, initial)) return dx12_fail(4, "initial client chrome/metrics differ");
  const auto native = std::get<Win32SurfaceHandle>((*window)->native_surface());
  if (native.composition) return dx12_fail(5, "ordinary client window silently became native Popup");
  client_regions(**window);
  if (!client_hits(**window)) return dx12_fail(6, "initial client hit regions differ");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  auto renderer = ordinary_renderer(**window, true); // Baseline must RED here, not bypass surface policy.
  if (!renderer) return dx12_fail(7, renderer.error().message.c_str());
  if (!ordinary_composition((*renderer)->surface_composition(), true))
    return dx12_fail(8, "initial client alpha does not report premultiplied composition");
  for (const bool blue : {false, true}) {
    if (!ordinary_draw_bands(**renderer, **window, blue) ||
        !ordinary_desktop_bands(**window, blue, true) || !title_row_pixels(**renderer, **window, blue))
      return dx12_fail(9, "self-drawn title row or client body lost GPU/desktop alpha");
    ClientGeometry observed;
    if (ordinary_hwnd(**window) != hwnd || !client_geometry(**window, observed) ||
        !same_geometry(initial, observed) || !client_hits(**window))
      return dx12_fail(10, "alpha presentation changed client chrome/geometry/hit regions");
  }
  return 0; // No HMENU is installed: this tracer does not claim native-menu compatibility.
}
