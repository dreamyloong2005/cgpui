#pragma once

#include "cgpui/platform/platform_window_title_bar.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <span>
#include <vector>

namespace cgpui {

// Physical-pixel frame metrics at one DPI.
struct Win32FrameMetricsPx {
  UINT dpi = 96;
  // SM_CXSIZEFRAME + SM_CXPADDEDBORDER: resize band thickness.
  int resize_border = 0;
  // AdjustWindowRectExForDpi(style): native title bar height
  // (caption plus top frame) and the side/bottom frame width.
  int caption_height = 0;
  int frame = 0;
  int button_width = 0;   // SM_CXSIZE
  int button_height = 0;  // SM_CYSIZE
};

[[nodiscard]] Win32FrameMetricsPx win32_frame_metrics_px(
    UINT dpi, DWORD style = WS_OVERLAPPEDWINDOW, DWORD extended_style = 0);

// Client rect for WM_NCCALCSIZE in client title bar mode: the caption is
// removed, the side and bottom frames stay non-client, and a maximized window
// is also inset at the top so no content is clipped off-screen.
[[nodiscard]] RECT win32_client_title_bar_rect(
    RECT proposed, int frame, bool maximized);

struct Win32TitleBarHitInput {
  POINT window_point{};  // physical, relative to the window rect origin
  SIZE window_size{};
  POINT client_point{};  // physical, relative to the client origin
  UINT dpi = 96;
  bool maximized = false;
  bool resizable = true;
  std::span<const WindowControlRegion> regions;
};

// WM_NCHITTEST result: resize edges first (not when maximized), then the
// regions from last to first (topmost painted wins), else HTCLIENT.
[[nodiscard]] LRESULT win32_title_bar_hit_test(
    const Win32TitleBarHitInput& input);

[[nodiscard]] bool win32_is_caption_button_hit(WPARAM hit);

// Per-window client title bar state.
struct Win32ClientTitleBar {
  UINT dpi = 96;
  std::vector<WindowControlRegion> regions;
  WPARAM pressed_button = HTNOWHERE;
  bool tracking_leave = false;
};

class Win32TitleBarMessageTarget {
 public:
  virtual ~Win32TitleBarMessageTarget() = default;
  // Handles WM_NCCALCSIZE, WM_NCHITTEST and the non-client mouse messages of
  // a client title bar. Returns false to fall through to DefWindowProc.
  virtual bool title_bar_message(
      HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam,
      LRESULT& result) = 0;
};

bool win32_window_proc_handle_title_bar(
    HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam,
    Win32TitleBarMessageTarget* window, LRESULT& result);

} // namespace cgpui
