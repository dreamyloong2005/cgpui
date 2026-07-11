#include "win32_input_coordinates_internal.hpp"

#include <windowsx.h>

namespace cgpui {
namespace {

Point logical_point(POINT point, DpiScale scale) {
  const float divisor = scale.value > 0.0F ? scale.value : 1.0F;
  return {
      .x = static_cast<float>(point.x) / divisor,
      .y = static_cast<float>(point.y) / divisor,
  };
}

Point logical_client_point_from_screen(
    HWND hwnd,
    POINT point,
    DpiScale scale) {
  if (hwnd != nullptr) {
    ScreenToClient(hwnd, &point);
  }
  return logical_point(point, scale);
}

} // namespace

Point win32_logical_client_point(LPARAM lparam, DpiScale scale) {
  return logical_point(
      POINT{.x = GET_X_LPARAM(lparam), .y = GET_Y_LPARAM(lparam)},
      scale);
}

Point win32_logical_client_point_from_screen(
    HWND hwnd,
    LPARAM lparam,
    DpiScale scale) {
  return logical_client_point_from_screen(
      hwnd,
      POINT{.x = GET_X_LPARAM(lparam), .y = GET_Y_LPARAM(lparam)},
      scale);
}

Point win32_logical_client_point_from_screen(
    HWND hwnd,
    POINTL point,
    DpiScale scale) {
  return logical_client_point_from_screen(
      hwnd,
      POINT{.x = static_cast<LONG>(point.x), .y = static_cast<LONG>(point.y)},
      scale);
}

} // namespace cgpui
