#include "cgpui/platform/platform.hpp"
#include "win32_window_backdrop_internal.hpp"
#include "win32_window_title_bar_internal.hpp"

#include <dwmapi.h>

#include <chrono>
#include <cmath>
#include <iostream>
#include <variant>
#include <vector>

namespace {

using cgpui::WindowControlArea;

int fail(int code, const char* message) {
  std::cerr << "win32 client title bar: " << message << '\n';
  return code;
}

struct Fixture {
  HWND hwnd = nullptr;
  cgpui::PlatformWindow* window = nullptr;
  std::vector<cgpui::PlatformEvent>* events = nullptr;
  UINT dpi = 96;

  [[nodiscard]] float scale() const { return static_cast<float>(dpi) / 96.0F; }
  [[nodiscard]] int frame() const {
    return GetSystemMetricsForDpi(SM_CXSIZEFRAME, dpi) +
        GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
  }
  [[nodiscard]] float client_width() const {
    RECT client{};
    GetClientRect(hwnd, &client);
    return static_cast<float>(client.right) / scale();
  }
  // Screen point of a logical client point.
  [[nodiscard]] POINT screen(float x, float y) const {
    POINT origin{};
    ClientToScreen(hwnd, &origin);
    return POINT{origin.x + static_cast<LONG>(std::lround(x * scale())),
                 origin.y + static_cast<LONG>(std::lround(y * scale()))};
  }
  [[nodiscard]] LRESULT hit(POINT point) const {
    return SendMessageW(hwnd, WM_NCHITTEST, 0, MAKELPARAM(point.x, point.y));
  }
  [[nodiscard]] LRESULT hit_window(int x, int y) const {
    RECT window{};
    GetWindowRect(hwnd, &window);
    return hit(POINT{window.left + x, window.top + y});
  }
  [[nodiscard]] SIZE window_size() const {
    RECT window{};
    GetWindowRect(hwnd, &window);
    return SIZE{window.right - window.left, window.bottom - window.top};
  }
};

void pump_until(const auto& done) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!done() && std::chrono::steady_clock::now() < deadline) {
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != FALSE) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
    Sleep(5);
  }
}

void set_regions(Fixture& fixture) {
  const float width = fixture.client_width();
  using cgpui::Rect;
  fixture.window->set_window_control_regions({
      {WindowControlArea::caption, Rect{{0, 0}, {width, 32}}},
      {WindowControlArea::client, Rect{{100, 4}, {80, 24}}},
      {WindowControlArea::minimize_button, Rect{{width - 138, 0}, {46, 32}}},
      {WindowControlArea::maximize_button, Rect{{width - 92, 0}, {46, 32}}},
      {WindowControlArea::close_button, Rect{{width - 46, 0}, {46, 32}}},
  });
}

void synthetic_dpi_change(const Fixture& fixture, UINT dpi) {
  RECT window{};
  GetWindowRect(fixture.hwnd, &window);
  SendMessageW(fixture.hwnd, WM_DPICHANGED, MAKEWPARAM(dpi, dpi),
               reinterpret_cast<LPARAM>(&window));
}

} // namespace

namespace {

int check_client_rect(const Fixture& fixture, int base) {
  RECT window{};
  GetWindowRect(fixture.hwnd, &window);
  RECT client{};
  GetClientRect(fixture.hwnd, &client);
  POINT origin{};
  ClientToScreen(fixture.hwnd, &origin);
  const int frame = fixture.frame();
  if (origin.x - window.left != frame || origin.y != window.top ||
      window.right - (origin.x + client.right) != frame ||
      window.bottom - (origin.y + client.bottom) != frame) {
    return fail(base, "client area must drop the caption and keep the frame");
  }
  RECT proposed{100, 100, 500, 400};
  SendMessageW(fixture.hwnd, WM_NCCALCSIZE, FALSE, reinterpret_cast<LPARAM>(&proposed));
  if (proposed.left != 100 + frame || proposed.top != 100 ||
      proposed.right != 500 - frame || proposed.bottom != 400 - frame) {
    return fail(base + 1, "WM_NCCALCSIZE must return the caption-less rect");
  }
  return 0;
}

int check_hits(const Fixture& fixture, int base) {
  const float width = fixture.client_width();
  struct Expected { float x; float y; LRESULT hit; };
  const Expected regions[]{
      {50, 16, HTCAPTION}, {120, 16, HTCLIENT}, {200, 150, HTCLIENT},
      {width - 115, 16, HTMINBUTTON}, {width - 69, 16, HTMAXBUTTON},
      {width - 23, 16, HTCLOSE}, {50, 40, HTCLIENT}};
  for (const Expected& expected : regions) {
    if (fixture.hit(fixture.screen(expected.x, expected.y)) != expected.hit) {
      std::cerr << "region hit at " << expected.x << ',' << expected.y << '\n';
      return fail(base, "region hit test mismatch");
    }
  }
  const SIZE size = fixture.window_size();
  const int frame = fixture.frame();
  const int w = size.cx;
  const int h = size.cy;
  struct Edge { int x; int y; LRESULT hit; };
  const Edge edges[]{
      {1, h / 2, HTLEFT}, {frame - 1, h / 2, HTLEFT}, {w - 1, h / 2, HTRIGHT},
      {w / 3, h - 1, HTBOTTOM}, {w / 3, 0, HTTOP}, {w / 3, frame - 1, HTTOP},
      {0, 0, HTTOPLEFT}, {w - 1, 0, HTTOPRIGHT}, {0, h - 1, HTBOTTOMLEFT},
      {w - 1, h - 1, HTBOTTOMRIGHT}};
  for (const Edge& edge : edges) {
    if (fixture.hit_window(edge.x, edge.y) != edge.hit) {
      std::cerr << "edge hit at " << edge.x << ',' << edge.y << " got "
                << fixture.hit_window(edge.x, edge.y) << '\n';
      return fail(base + 1, "resize edge hit test mismatch");
    }
  }
  // Just below the top band the caption region takes over again.
  RECT window{};
  GetWindowRect(fixture.hwnd, &window);
  if (fixture.hit(POINT{fixture.screen(50, 0).x, window.top + frame}) != HTCAPTION) {
    return fail(base + 2, "the caption must start below the top resize band");
  }
  return 0;
}

void print_metrics(const Fixture& fixture) {
  const cgpui::PlatformWindowFrameMetrics metrics = fixture.window->frame_metrics();
  std::cout << "dpi " << fixture.dpi << ": frame px " << fixture.frame()
            << ", caption_height " << metrics.caption_height
            << ", resize_border " << metrics.resize_border
            << ", button " << metrics.caption_button_size.width << 'x'
            << metrics.caption_button_size.height
            << ", client top band " << metrics.client_resize_border.top << '\n';
}

int check_metrics(const Fixture& fixture, int base) {
  print_metrics(fixture);
  const cgpui::PlatformWindowFrameMetrics metrics = fixture.window->frame_metrics();
  RECT frame{};
  AdjustWindowRectExForDpi(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0, fixture.dpi);
  const float scale = fixture.scale();
  if (!metrics.supported || !metrics.client_title_bar ||
      std::fabs(metrics.resize_border - static_cast<float>(fixture.frame()) / scale) > 0.01F ||
      std::fabs(metrics.caption_height - static_cast<float>(-frame.top) / scale) > 0.01F ||
      std::fabs(metrics.client_resize_border.top - metrics.resize_border) > 0.01F) {
    return fail(base, "frame metrics must follow the window DPI");
  }
  return 0;
}

} // namespace

namespace {

int check_forwarding(Fixture& fixture, int base) {
  const float width = fixture.client_width();
  const POINT max_center = fixture.screen(width - 69, 16);
  const LPARAM at = MAKELPARAM(max_center.x, max_center.y);
  fixture.events->clear();
  SendMessageW(fixture.hwnd, WM_NCMOUSEMOVE, HTMAXBUTTON, at);
  const auto* moved = fixture.events->empty()
      ? nullptr : std::get_if<cgpui::PointerMoved>(&fixture.events->back());
  if (moved == nullptr || std::fabs(moved->position.x - (width - 69)) > 1.0F ||
      std::fabs(moved->position.y - 16) > 1.0F) {
    return fail(base, "WM_NCMOUSEMOVE over HTMAXBUTTON must reach the app as a move");
  }
  SendMessageW(fixture.hwnd, WM_NCLBUTTONDOWN, HTMAXBUTTON, at);
  SendMessageW(fixture.hwnd, WM_NCLBUTTONUP, HTMAXBUTTON, at);
  int presses = 0;
  int releases = 0;
  for (const auto& event : *fixture.events) {
    if (const auto* button = std::get_if<cgpui::PointerButton>(&event)) {
      (button->pressed ? presses : releases) += 1;
    }
  }
  if (presses != 1 || releases != 1) {
    return fail(base + 1, "caption button presses must reach the app");
  }
  pump_until([&] { return IsZoomed(fixture.hwnd) != FALSE; });
  if (IsZoomed(fixture.hwnd) == FALSE) {
    return fail(base + 2, "releasing over HTMAXBUTTON must maximize");
  }
  return 0;
}

int check_maximized(Fixture& fixture, int base) {
  RECT window{};
  GetWindowRect(fixture.hwnd, &window);
  POINT origin{};
  ClientToScreen(fixture.hwnd, &origin);
  RECT client{};
  GetClientRect(fixture.hwnd, &client);
  MONITORINFO monitor{.cbSize = sizeof(MONITORINFO)};
  GetMonitorInfoW(MonitorFromWindow(fixture.hwnd, MONITOR_DEFAULTTONEAREST), &monitor);
  const int frame = fixture.frame();
  std::cout << "maximized: window top " << window.top << ", client top " << origin.y
            << ", work top " << monitor.rcWork.top << '\n';
  if (origin.y - window.top != frame || origin.x - window.left != frame ||
      origin.x < monitor.rcWork.left || origin.y < monitor.rcWork.top ||
      origin.x + client.right > monitor.rcWork.right ||
      origin.y + client.bottom > monitor.rcWork.bottom) {
    return fail(base, "a maximized window must be inset by the frame on every edge");
  }
  set_regions(fixture);
  if (fixture.hit(fixture.screen(50, 1)) != HTCAPTION ||
      fixture.window->frame_metrics().client_resize_border.top != 0.0F) {
    return fail(base + 1, "a maximized window has no top resize band");
  }
  const POINT max_center = fixture.screen(fixture.client_width() - 69, 16);
  const LPARAM at = MAKELPARAM(max_center.x, max_center.y);
  SendMessageW(fixture.hwnd, WM_NCLBUTTONDOWN, HTMAXBUTTON, at);
  SendMessageW(fixture.hwnd, WM_NCLBUTTONUP, HTMAXBUTTON, at);
  pump_until([&] { return IsZoomed(fixture.hwnd) == FALSE; });
  return IsZoomed(fixture.hwnd) == FALSE
      ? 0 : fail(base + 2, "releasing over HTMAXBUTTON again must restore");
}

int check_backdrop_plan() {
  const auto plan = cgpui::win32_plan_window_backdrop({
      .requested = cgpui::WindowBackdrop::mica, .transparent_surface = false,
      .os_build = 26100, .client_title_bar = true});
  return plan.state.title_bar != cgpui::WindowBackdrop::none ||
          plan.extend_frame || plan.state.reason.empty()
      ? fail(5, "an opaque client title bar window shows no material")
      : 0;
}

} // namespace

int main() {
  if (const int code = check_backdrop_plan(); code != 0) return code;
  auto app = cgpui::create_platform_application();
  if (!app) return fail(10, "platform application was not created");
  std::vector<cgpui::PlatformEvent> events;
  cgpui::WindowDescriptor descriptor;
  descriptor.title = "CGPUI client title bar test";
  descriptor.size = cgpui::Size{.width = 480.0F, .height = 320.0F};
  descriptor.chrome.title_bar = cgpui::WindowTitleBarMode::client;
  auto window = (*app)->create_window(
      descriptor, [&](const cgpui::PlatformEvent& event) { events.push_back(event); });
  if (!window) return fail(11, "window was not created");
  const cgpui::NativeSurfaceHandle native = (*window)->native_surface();
  const auto* surface = std::get_if<cgpui::Win32SurfaceHandle>(&native);
  if (surface == nullptr || surface->hwnd == nullptr) return fail(12, "no HWND");
  Fixture fixture{
      .hwnd = static_cast<HWND>(surface->hwnd),
      .window = window->get(),
      .events = &events,
      .dpi = GetDpiForWindow(static_cast<HWND>(surface->hwnd))};
  if (((*window)->apply_window_chrome(descriptor.chrome).applied.title_bar !=
       cgpui::WindowTitleBarMode::client) ||
      (GetWindowLongPtrW(fixture.hwnd, GWL_STYLE) & WS_CAPTION) != WS_CAPTION) {
    return fail(13, "client mode must keep WS_CAPTION");
  }
  RECT buttons{};
  const HRESULT bounds = DwmGetWindowAttribute(
      fixture.hwnd, DWMWA_CAPTION_BUTTON_BOUNDS, &buttons, sizeof(buttons));
  std::cout << "DWMWA_CAPTION_BUTTON_BOUNDS hr 0x" << std::hex << bounds << std::dec
            << " rect " << buttons.left << ',' << buttons.top << ','
            << buttons.right << ',' << buttons.bottom << '\n';

  // Native DPI first: maximize uses the real frame, so check it there.
  set_regions(fixture);
  if (const int code = check_client_rect(fixture, 20); code != 0) return code;
  if (const int code = check_forwarding(fixture, 30); code != 0) return code;
  if (const int code = check_maximized(fixture, 40); code != 0) return code;

  for (const UINT dpi : {96U, 144U}) {
    synthetic_dpi_change(fixture, dpi);
    fixture.dpi = dpi;
    set_regions(fixture);
    const int base = dpi == 96U ? 50 : 70;
    if (const int code = check_client_rect(fixture, base); code != 0) return code;
    if (const int code = check_hits(fixture, base + 5); code != 0) return code;
    if (const int code = check_metrics(fixture, base + 10); code != 0) return code;
  }

  // Close goes through WM_SYSCOMMAND(SC_CLOSE) -> WM_CLOSE -> close policy.
  events.clear();
  const POINT close = fixture.screen(fixture.client_width() - 23, 16);
  const LPARAM at = MAKELPARAM(close.x, close.y);
  SendMessageW(fixture.hwnd, WM_NCLBUTTONDOWN, HTCLOSE, at);
  SendMessageW(fixture.hwnd, WM_NCLBUTTONUP, HTCLOSE, at);
  const auto close_seen = [&] {
    for (const auto& event : events) {
      if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) return true;
    }
    return false;
  };
  pump_until(close_seen);
  if (!close_seen()) return fail(90, "HTCLOSE release must request close");

  // Back to the system title bar: WM_NCCALCSIZE is the default again.
  cgpui::WindowChromeOptions system = descriptor.chrome;
  system.title_bar = cgpui::WindowTitleBarMode::system;
  (void)(*window)->apply_window_chrome(system);
  POINT origin{};
  ClientToScreen(fixture.hwnd, &origin);
  RECT rect{};
  GetWindowRect(fixture.hwnd, &rect);
  if (origin.y == rect.top || (*window)->frame_metrics().client_title_bar) {
    return fail(91, "system mode must restore the native caption");
  }
  return 0;
}
