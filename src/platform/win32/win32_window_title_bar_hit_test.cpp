#include "win32_window_title_bar_internal.hpp"

namespace cgpui {
namespace {

LRESULT hit_for_area(WindowControlArea area) {
  switch (area) {
    case WindowControlArea::caption:
      return HTCAPTION;
    case WindowControlArea::minimize_button:
      return HTMINBUTTON;
    case WindowControlArea::maximize_button:
      return HTMAXBUTTON;
    case WindowControlArea::close_button:
      return HTCLOSE;
    case WindowControlArea::client:
      break;
  }
  return HTCLIENT;
}

LRESULT resize_edge_hit(const Win32TitleBarHitInput& input, int border) {
  const bool left = input.window_point.x < border;
  const bool right = input.window_point.x >= input.window_size.cx - border;
  const bool top = input.window_point.y < border;
  const bool bottom = input.window_point.y >= input.window_size.cy - border;
  if (top) return left ? HTTOPLEFT : right ? HTTOPRIGHT : HTTOP;
  if (bottom) return left ? HTBOTTOMLEFT : right ? HTBOTTOMRIGHT : HTBOTTOM;
  if (left) return HTLEFT;
  if (right) return HTRIGHT;
  return HTNOWHERE;
}

} // namespace

Win32FrameMetricsPx win32_frame_metrics_px(
    UINT dpi, DWORD style, DWORD extended_style) {
  Win32FrameMetricsPx metrics{
      .dpi = dpi,
      .resize_border = GetSystemMetricsForDpi(SM_CXSIZEFRAME, dpi) +
          GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi),
      .button_width = GetSystemMetricsForDpi(SM_CXSIZE, dpi),
      .button_height = GetSystemMetricsForDpi(SM_CYSIZE, dpi),
  };
  RECT frame{};
  if (AdjustWindowRectExForDpi(&frame, style, FALSE, extended_style, dpi) != FALSE) {
    metrics.caption_height = -frame.top;
    metrics.frame = -frame.left;
  }
  return metrics;
}

RECT win32_client_title_bar_rect(RECT proposed, int frame, bool maximized) {
  proposed.left += frame;
  proposed.right -= frame;
  proposed.bottom -= frame;
  if (maximized) proposed.top += frame;
  return proposed;
}

bool win32_is_caption_button_hit(WPARAM hit) {
  return hit == HTMINBUTTON || hit == HTMAXBUTTON || hit == HTCLOSE;
}

LRESULT win32_title_bar_hit_test(const Win32TitleBarHitInput& input) {
  if (!input.maximized && input.resizable) {
    const int border = win32_frame_metrics_px(input.dpi).resize_border;
    if (const LRESULT edge = resize_edge_hit(input, border); edge != HTNOWHERE) {
      return edge;
    }
  }
  const float scale = static_cast<float>(input.dpi) / 96.0F;
  const Point point{
      .x = static_cast<float>(input.client_point.x) / scale,
      .y = static_cast<float>(input.client_point.y) / scale,
  };
  for (auto region = input.regions.rbegin(); region != input.regions.rend();
       ++region) {
    if (contains(region->bounds, point)) return hit_for_area(region->area);
  }
  return HTCLIENT;
}

} // namespace cgpui
