// Win32 popup windows (WindowKind::popup), the cases of
// macos_popup_window_test: support probe, no popup without a parent,
// style bits, owner, shown without activation, placement in the parent's
// client coordinates, a flip at the monitor's work area edge, following the
// parent, and pointer transparency (layered and transparent: the window
// below gets the pointer).
#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cmath>
#include <iostream>

namespace {

using cgpui::Rect;
using cgpui::WindowPopupSide;

void pump() {
  for (int round = 0; round < 5; ++round) {
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
    Sleep(10);
  }
}

HWND hwnd_of(const cgpui::PlatformWindow& window) {
  return static_cast<HWND>(std::get<cgpui::Win32SurfaceHandle>(window.native_surface()).hwnd);
}

bool close_to(float a, float b) { return std::fabs(a - b) <= 1.0F; }

bool same(Rect rect, float x, float y, float width, float height) {
  const bool same = close_to(rect.origin.x, x) && close_to(rect.origin.y, y) &&
                    close_to(rect.size.width, width) && close_to(rect.size.height, height);
  if (!same) {
    std::cerr << "bounds " << rect.origin.x << ' ' << rect.origin.y << ' ' << rect.size.width
              << ' ' << rect.size.height << ", expected " << x << ' ' << y << ' ' << width
              << ' ' << height << '\n';
  }
  return same;
}

POINT client_origin(HWND hwnd) {
  POINT origin{};
  ClientToScreen(hwnd, &origin);
  return origin;
}

cgpui::WindowDescriptor popup_descriptor(bool pointer_transparent, Rect anchor) {
  cgpui::WindowDescriptor descriptor{
      .size = {120.0F, 80.0F},
      .kind = cgpui::WindowKind::popup,
      .popup = cgpui::WindowPopupPlacement{
          .anchor = anchor, .side = WindowPopupSide::below, .gap = 4}};
  descriptor.popup_options.pointer_transparent = pointer_transparent;
  return descriptor;
}

} // namespace

int main() {
  auto application = cgpui::create_platform_application();
  if (!application || !(*application)->popup_windows_supported()) return 10;
  const auto ignore = [](const cgpui::PlatformEvent&) {};
  auto parent = (*application)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Win32 Popup Test", .size = {400.0F, 300.0F}}, ignore);
  if (!parent) return 11;
  const HWND parent_hwnd = hwnd_of(**parent);
  // Topmost (its popups too), so windows of tests running alongside stay below.
  SetWindowPos(parent_hwnd, HWND_TOPMOST, 200, 200, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
  pump();
  const Rect anchor{{10, 10}, {40, 20}};
  if ((*application)->create_window(popup_descriptor(false, anchor), ignore)) {
    return 12;  // a popup needs a parent
  }

  const HWND active_before = GetActiveWindow();
  auto popup =
      (*application)->create_child_window(popup_descriptor(false, anchor), **parent, ignore);
  if (!popup) return 13;
  pump();
  const HWND popup_hwnd = hwnd_of(**popup);
  const LONG_PTR style = GetWindowLongPtrW(popup_hwnd, GWL_STYLE);
  const LONG_PTR extended = GetWindowLongPtrW(popup_hwnd, GWL_EXSTYLE);
  constexpr LONG_PTR popup_extended = WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_NOREDIRECTIONBITMAP;
  if ((style & WS_POPUP) == 0 || (style & (WS_CAPTION | WS_THICKFRAME | WS_CHILD)) != 0 ||
      (extended & popup_extended) != popup_extended ||
      (extended & (WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_APPWINDOW)) != 0) {
    return 14;
  }
  if (GetWindow(popup_hwnd, GW_OWNER) != parent_hwnd || IsWindowVisible(popup_hwnd) == FALSE ||
      GetActiveWindow() != active_before || GetActiveWindow() == popup_hwnd ||
      GetForegroundWindow() == popup_hwnd ||
      SendMessageW(popup_hwnd, WM_MOUSEACTIVATE, reinterpret_cast<WPARAM>(parent_hwnd),
                   MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN)) != MA_NOACTIVATE) {
    return 15;
  }
  const auto popup_surface = std::get<cgpui::Win32SurfaceHandle>((*popup)->native_surface());
  const auto parent_surface = std::get<cgpui::Win32SurfaceHandle>((*parent)->native_surface());
  if (!popup_surface.composition || parent_surface.composition) return 16;
  const float scale = (*parent)->state().scale.value;
  if (!close_to((*popup)->state().framebuffer_size.width, std::lround(120.0F * scale))) return 17;

  // Below the anchor, in the parent's client coordinates and on screen.
  const auto placed = (*popup)->place_popup(
      {.anchor = {{10, 10}, {40, 20}}, .side = WindowPopupSide::below, .gap = 4});
  RECT rect{};
  GetWindowRect(popup_hwnd, &rect);
  POINT origin = client_origin(parent_hwnd);
  std::cout << "scale " << scale << ", below: " << placed.bounds.origin.x << ' '
            << placed.bounds.origin.y << ' ' << placed.bounds.size.width << ' '
            << placed.bounds.size.height << ", screen " << rect.left << ',' << rect.top << '\n';
  if (!placed.supported || !placed.placed || placed.flipped ||
      !same(placed.bounds, 10, 34, 120, 80) ||
      std::abs(rect.left - (origin.x + std::lround(10 * scale))) > 1 ||
      std::abs(rect.top - (origin.y + std::lround(34 * scale))) > 1) {
    return 18;
  }
  // Owned windows do not follow their owner on Windows; the popup does.
  SetWindowPos(parent_hwnd, nullptr, 230, 220, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
  pump();
  RECT moved{};
  GetWindowRect(popup_hwnd, &moved);
  std::cout << "parent moved by 30,20: popup moved by " << moved.left - rect.left << ','
            << moved.top - rect.top << '\n';
  if (moved.left - rect.left != 30 || moved.top - rect.top != 20) return 19;

  // At the bottom of the work area there is no room below: flip above.
  MONITORINFO monitor{.cbSize = sizeof(MONITORINFO)};
  GetMonitorInfoW(MonitorFromWindow(parent_hwnd, MONITOR_DEFAULTTONEAREST), &monitor);
  RECT client{};
  GetClientRect(parent_hwnd, &client);
  RECT window{};
  GetWindowRect(parent_hwnd, &window);
  origin = client_origin(parent_hwnd);
  const LONG lift = (origin.y - window.top) + client.bottom + 10;  // client bottom 10 px above
  SetWindowPos(parent_hwnd, nullptr, monitor.rcWork.left + 100, monitor.rcWork.bottom - lift, 0, 0,
               SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
  pump();
  const float client_height = static_cast<float>(client.bottom) / scale;
  const auto flipped = (*popup)->place_popup(
      {.anchor = {{10, client_height - 30}, {40, 20}}, .side = WindowPopupSide::below, .gap = 4});
  std::cout << "below at the work area bottom: flipped " << flipped.flipped << ", "
            << flipped.bounds.origin.x << ' ' << flipped.bounds.origin.y << '\n';
  if (!flipped.placed || !flipped.flipped ||
      !same(flipped.bounds, 10, client_height - 30 - 4 - 80, 120, 80)) {
    return 20;
  }
  SetWindowPos(parent_hwnd, nullptr, 200, 200, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
  pump();

  // A pointer-transparent popup leaves the pointer to the window below
  // (the parent here; the other popup is elsewhere); an ordinary popup
  // takes it.
  auto tooltip = (*application)->create_child_window(
      popup_descriptor(true, Rect{{200, 140}, {0, 0}}), **parent, ignore);
  if (!tooltip) return 21;
  pump();
  const HWND tooltip_hwnd = hwnd_of(**tooltip);
  const LONG_PTR tooltip_extended = GetWindowLongPtrW(tooltip_hwnd, GWL_EXSTYLE);
  GetWindowRect(tooltip_hwnd, &rect);
  const POINT tooltip_center{(rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2};
  if ((tooltip_extended & (WS_EX_LAYERED | WS_EX_TRANSPARENT)) !=
          (WS_EX_LAYERED | WS_EX_TRANSPARENT) ||
      WindowFromPoint(tooltip_center) != parent_hwnd) {
    return 22;
  }
  std::cout << "pointer-transparent popup: the window at its center is the parent" << '\n';
  tooltip->reset();
  pump();
  GetWindowRect(popup_hwnd, &rect);
  if (WindowFromPoint(POINT{(rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2}) !=
      popup_hwnd) {
    return 23;
  }
  // Normal windows are not popups.
  return !(*parent)->place_popup({}).supported ? 0 : 24;
}
