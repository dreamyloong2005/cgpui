#include "win32_window_internal.hpp"

namespace cgpui {
namespace {

// SC_SIZE + WMSZ_* starts an interactive resize from that edge; SC_MOVE +
// HTCAPTION starts an interactive move (the "SC_DRAGMOVE" form).
WPARAM resize_command(WindowResizeEdge edge) {
  switch (edge) {
    case WindowResizeEdge::left: return SC_SIZE + WMSZ_LEFT;
    case WindowResizeEdge::right: return SC_SIZE + WMSZ_RIGHT;
    case WindowResizeEdge::top: return SC_SIZE + WMSZ_TOP;
    case WindowResizeEdge::top_left: return SC_SIZE + WMSZ_TOPLEFT;
    case WindowResizeEdge::top_right: return SC_SIZE + WMSZ_TOPRIGHT;
    case WindowResizeEdge::bottom: return SC_SIZE + WMSZ_BOTTOM;
    case WindowResizeEdge::bottom_left: return SC_SIZE + WMSZ_BOTTOMLEFT;
    case WindowResizeEdge::bottom_right: break;
  }
  return SC_SIZE + WMSZ_BOTTOMRIGHT;
}

void enable_menu_item(HMENU menu, UINT item, bool enabled) {
  EnableMenuItem(menu, item, MF_BYCOMMAND | (enabled ? MF_ENABLED : MF_GRAYED));
}

PlatformWindowActionResult post_command(HWND hwnd, WPARAM command) {
  ReleaseCapture();
  const bool posted = PostMessageW(hwnd, WM_SYSCOMMAND, command, 0) != FALSE;
  return PlatformWindowActionResult{
      .supported = true, .performed = posted,
      .reason = posted ? std::string{} : "PostMessageW(WM_SYSCOMMAND) failed"};
}

} // namespace

PlatformWindowFrameMetrics Win32Window::frame_metrics() const {
  const Win32FrameMetricsPx px = win32_frame_metrics_px(title_bar_.dpi);
  const float scale = static_cast<float>(px.dpi) / 96.0F;
  const bool client = client_title_bar_active();
  const bool maximized = hwnd_ != nullptr && IsZoomed(hwnd_) != FALSE;
  const float border = static_cast<float>(px.resize_border) / scale;
  return PlatformWindowFrameMetrics{
      .supported = true,
      .client_title_bar = client,
      .backend = "win32",
      .caption_height = static_cast<float>(px.caption_height) / scale,
      .caption_button_size = Size{
          static_cast<float>(px.button_width) / scale,
          static_cast<float>(px.button_height) / scale},
      .resize_border = border,
      .client_resize_border = WindowFrameInsets{
          .top = client && !maximized && chrome_state_.platform.applied.resizable
              ? border : 0.0F},
      .mechanism = "WM_NCCALCSIZE + WM_NCHITTEST; GetSystemMetricsForDpi("
                   "SM_CXSIZEFRAME + SM_CXPADDEDBORDER) at " +
          std::to_string(px.dpi) + " DPI",
  };
}

PlatformWindowActionResult Win32Window::perform_window_action(
    PlatformWindowActionRequest request) {
  if (hwnd_ == nullptr || IsWindow(hwnd_) == FALSE) {
    return PlatformWindowActionResult{.reason = "window has no HWND"};
  }
  switch (request.action) {
    case PlatformWindowAction::begin_move:
      return post_command(hwnd_, SC_MOVE + HTCAPTION);
    case PlatformWindowAction::begin_resize:
      return post_command(hwnd_, resize_command(request.edge));
    case PlatformWindowAction::show_window_menu: {
      HMENU menu = GetSystemMenu(hwnd_, FALSE);
      if (menu == nullptr) {
        return PlatformWindowActionResult{.reason = "window has no system menu"};
      }
      const bool zoomed = IsZoomed(hwnd_) != FALSE;
      const bool resizable = chrome_state_.platform.applied.resizable;
      enable_menu_item(menu, SC_RESTORE, zoomed);
      enable_menu_item(menu, SC_MOVE, !zoomed);
      enable_menu_item(menu, SC_SIZE, !zoomed && resizable);
      enable_menu_item(menu, SC_MAXIMIZE, !zoomed && resizable);
      enable_menu_item(menu, SC_MINIMIZE, true);
      const float scale = static_cast<float>(title_bar_.dpi) / 96.0F;
      POINT screen{
          static_cast<LONG>(request.position.x * scale),
          static_cast<LONG>(request.position.y * scale)};
      ClientToScreen(hwnd_, &screen);
      const BOOL command = TrackPopupMenu(
          menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, screen.x, screen.y, 0, hwnd_,
          nullptr);
      if (command != 0) PostMessageW(hwnd_, WM_SYSCOMMAND, command, 0);
      return PlatformWindowActionResult{.supported = true, .performed = true};
    }
    case PlatformWindowAction::minimize:
    case PlatformWindowAction::toggle_maximize:
    case PlatformWindowAction::close:
      break;
  }
  return PlatformWindow::perform_window_action(request);
}

} // namespace cgpui
