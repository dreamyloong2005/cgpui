#include "win32_window_internal.hpp"

#include "win32_input_coordinates_internal.hpp"

namespace cgpui {
namespace {

POINT screen_point(LPARAM lparam) {
  return POINT{
      static_cast<short>(LOWORD(lparam)),
      static_cast<short>(HIWORD(lparam))};
}

WPARAM button_command(HWND hwnd, WPARAM hit) {
  if (hit == HTMINBUTTON) return SC_MINIMIZE;
  if (hit == HTMAXBUTTON) return IsZoomed(hwnd) != FALSE ? SC_RESTORE : SC_MAXIMIZE;
  return SC_CLOSE;
}

} // namespace

bool Win32Window::client_title_bar_active() const {
  const WindowChromeOptions& applied = chrome_state_.platform.applied;
  return applied.title_bar == WindowTitleBarMode::client &&
      applied.decorations && !display_command_state_.fullscreen;
}

void Win32Window::set_window_control_regions(
    std::vector<WindowControlRegion> regions) {
  title_bar_.regions = std::move(regions);
}

bool Win32Window::title_bar_message(
    HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result) {
  if (!client_title_bar_active()) return false;
  const bool button = win32_is_caption_button_hit(wparam);
  switch (message) {
    case WM_NCCALCSIZE: {
      RECT* rect = wparam != FALSE
          ? &reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam)->rgrc[0]
          : reinterpret_cast<RECT*>(lparam);
      const int frame = win32_frame_metrics_px(
          title_bar_.dpi, chrome_state_.style, chrome_state_.extended_style).frame;
      *rect = win32_client_title_bar_rect(*rect, frame, IsZoomed(hwnd) != FALSE);
      result = 0;
      return true;
    }
    case WM_NCHITTEST: {
      const POINT screen = screen_point(lparam);
      RECT window{};
      GetWindowRect(hwnd, &window);
      POINT client = screen;
      ScreenToClient(hwnd, &client);
      result = win32_title_bar_hit_test(Win32TitleBarHitInput{
          .window_point = {screen.x - window.left, screen.y - window.top},
          .window_size = {window.right - window.left, window.bottom - window.top},
          .client_point = client,
          .dpi = title_bar_.dpi,
          .maximized = IsZoomed(hwnd) != FALSE,
          .resizable = chrome_state_.platform.applied.resizable,
          .regions = title_bar_.regions,
      });
      return true;
    }
    case WM_NCMOUSEMOVE:
      // The caption row is non-client to Windows, so the app sees no
      // WM_MOUSEMOVE there; forward it so hover state stays correct.
      if (wparam == HTCAPTION || button) {
        callback_(PointerMoved{.position = win32_logical_client_point_from_screen(
                                   hwnd, lparam, state_.scale)});
        if (!title_bar_.tracking_leave) {
          TRACKMOUSEEVENT track{
              .cbSize = sizeof(TRACKMOUSEEVENT),
              .dwFlags = TME_LEAVE | TME_NONCLIENT,
              .hwndTrack = hwnd};
          title_bar_.tracking_leave = TrackMouseEvent(&track) != FALSE;
        }
      }
      return false;
    case WM_NCMOUSELEAVE: {
      title_bar_.tracking_leave = false;
      title_bar_.pressed_button = HTNOWHERE;
      POINT cursor{};
      RECT window{};
      GetCursorPos(&cursor);
      GetWindowRect(hwnd, &window);
      if (PtInRect(&window, cursor) == FALSE) {
        callback_(PointerExited{.position = win32_logical_client_point_from_screen(
                                    hwnd, MAKELPARAM(cursor.x, cursor.y), state_.scale)});
      }
      return false;
    }
    case WM_NCLBUTTONDOWN:
    case WM_NCLBUTTONDBLCLK:
    case WM_NCLBUTTONUP: {
      // DefWindowProc would track and draw classic caption buttons; the app
      // draws them, so the press goes to the app and the action runs here.
      if (!button) return false;
      const bool pressed = message != WM_NCLBUTTONUP;
      callback_(PointerButton{
          .button = MouseButton::left,
          .pressed = pressed,
          .click_count = static_cast<std::uint8_t>(message == WM_NCLBUTTONDBLCLK ? 2 : 1),
          .position = win32_logical_client_point_from_screen(hwnd, lparam, state_.scale)});
      if (pressed) {
        title_bar_.pressed_button = wparam;
      } else if (title_bar_.pressed_button == wparam) {
        title_bar_.pressed_button = HTNOWHERE;
        PostMessageW(hwnd, WM_SYSCOMMAND, button_command(hwnd, wparam), 0);
      }
      result = 0;
      return true;
    }
    default:
      return false;
  }
}

} // namespace cgpui
