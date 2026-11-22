#include "win32_popup_window_internal.hpp"
#include "../platform_popup_placement_internal.hpp"

#include <cmath>

namespace cgpui {
namespace {

float owner_scale(HWND owner) {
  const UINT dpi = GetDpiForWindow(owner);
  return dpi == 0 ? 1.0F : static_cast<float>(dpi) / 96.0F;
}

POINT client_origin(HWND owner) {
  POINT origin{};
  ClientToScreen(owner, &origin);
  return origin;
}

Rect screen_rect(const RECT& rect) {
  return Rect{{static_cast<float>(rect.left), static_cast<float>(rect.top)},
              {static_cast<float>(rect.right - rect.left),
               static_cast<float>(rect.bottom - rect.top)}};
}

void move_to(HWND popup, POINT origin) {
  RECT current{};
  GetWindowRect(popup, &current);
  // An unchanged origin sends no WM_MOVE (the runtime may lay out again).
  if (current.left == origin.x && current.top == origin.y) return;
  SetWindowPos(popup, nullptr, origin.x, origin.y, 0, 0,
               SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
}

BOOL CALLBACK follow_owner(HWND hwnd, LPARAM owner_value) {
  const auto owner = reinterpret_cast<HWND>(owner_value);
  if (GetWindow(hwnd, GW_OWNER) != owner || !win32_is_popup_window(hwnd) ||
      (GetWindowLongW(hwnd, win32_popup_flags_index) & win32_popup_placed) == 0) {
    return TRUE;
  }
  const POINT origin = client_origin(owner);
  move_to(hwnd, POINT{origin.x + GetWindowLongW(hwnd, win32_popup_offset_x_index),
                      origin.y + GetWindowLongW(hwnd, win32_popup_offset_y_index)});
  return TRUE;
}

} // namespace

// The anchor goes from the owner's logical client pixels to physical
// screen pixels (owner scale, ClientToScreen); the popup is placed on the
// work area of the monitor nearest the anchor's center and reported back
// in the owner's logical client pixels.
PlatformPopupPlacementResult win32_place_popup(
    HWND popup, WindowPopupPlacement placement) {
  PlatformPopupPlacementResult result{.supported = win32_is_popup_window(popup)};
  const HWND owner = popup == nullptr ? nullptr : GetWindow(popup, GW_OWNER);
  if (!result.supported || owner == nullptr || IsWindow(owner) == FALSE) {
    result.reason = result.supported ? "popup parent window closed" : "not a popup window";
    return result;
  }
  const float scale = owner_scale(owner);
  const POINT origin = client_origin(owner);
  const Rect anchor{
      {static_cast<float>(origin.x) + placement.anchor.origin.x * scale,
       static_cast<float>(origin.y) + placement.anchor.origin.y * scale},
      {placement.anchor.size.width * scale, placement.anchor.size.height * scale}};
  const POINT center{
      static_cast<LONG>(std::lround(anchor.origin.x + anchor.size.width / 2.0F)),
      static_cast<LONG>(std::lround(anchor.origin.y + anchor.size.height / 2.0F))};
  MONITORINFO monitor{.cbSize = sizeof(MONITORINFO)};
  RECT window{};
  if (GetMonitorInfoW(MonitorFromPoint(center, MONITOR_DEFAULTTONEAREST), &monitor) == FALSE ||
      GetWindowRect(popup, &window) == FALSE) {
    result.reason = "GetMonitorInfoW or GetWindowRect failed";
    return result;
  }
  const PlatformPopupFrame frame = platform_popup_frame(
      anchor, screen_rect(window).size, placement.side, placement.gap * scale,
      screen_rect(monitor.rcWork));
  const POINT placed{static_cast<LONG>(std::lround(frame.bounds.origin.x)),
                     static_cast<LONG>(std::lround(frame.bounds.origin.y))};
  move_to(popup, placed);
  // A move to a monitor of another scale resized (and may have moved) the
  // popup on WM_DPICHANGED; what it got is what the owner keeps.
  GetWindowRect(popup, &window);
  const Rect bounds = screen_rect(window);
  SetWindowLongW(popup, win32_popup_offset_x_index, window.left - origin.x);
  SetWindowLongW(popup, win32_popup_offset_y_index, window.top - origin.y);
  SetWindowLongW(popup, win32_popup_flags_index,
                 GetWindowLongW(popup, win32_popup_flags_index) | win32_popup_placed);
  result.placed = true;
  result.flipped = frame.flipped;
  result.shifted = frame.shifted;
  result.bounds = Rect{
      {(bounds.origin.x - static_cast<float>(origin.x)) / scale,
       (bounds.origin.y - static_cast<float>(origin.y)) / scale},
      {bounds.size.width / scale, bounds.size.height / scale}};
  return result;
}

void win32_move_owned_popups(HWND owner) {
  EnumThreadWindows(GetCurrentThreadId(), follow_owner, reinterpret_cast<LPARAM>(owner));
}

} // namespace cgpui
