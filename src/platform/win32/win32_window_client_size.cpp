#include "win32_window_client_size_internal.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {
namespace {

LONG physical_extent(float logical, UINT dpi) {
  if (!std::isfinite(logical) || logical <= 0.0F) return 1;
  const float scale = static_cast<float>(dpi == 0 ? 96U : dpi) / 96.0F;
  return std::max<LONG>(1, std::lround(logical * scale));
}

} // namespace

SIZE win32_physical_client_size(Size logical, UINT dpi) {
  return SIZE{
      physical_extent(logical.width, dpi),
      physical_extent(logical.height, dpi)};
}

SIZE win32_outer_size_for_client(
    SIZE client, RECT window_rect, RECT client_rect) {
  const LONG frame_width = (window_rect.right - window_rect.left) -
      (client_rect.right - client_rect.left);
  const LONG frame_height = (window_rect.bottom - window_rect.top) -
      (client_rect.bottom - client_rect.top);
  return SIZE{client.cx + frame_width, client.cy + frame_height};
}

bool win32_set_logical_client_size(HWND hwnd, Size logical) {
  RECT window_rect{};
  RECT client_rect{};
  if (hwnd == nullptr || GetWindowRect(hwnd, &window_rect) == FALSE ||
      GetClientRect(hwnd, &client_rect) == FALSE) {
    return false;
  }
  const SIZE client = win32_physical_client_size(logical, GetDpiForWindow(hwnd));
  const SIZE outer = win32_outer_size_for_client(client, window_rect, client_rect);
  return SetWindowPos(
             hwnd, nullptr, 0, 0, outer.cx, outer.cy,
             SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE) != FALSE;
}

} // namespace cgpui
