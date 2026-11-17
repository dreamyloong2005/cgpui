#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <iostream>
#include <variant>

namespace {

int fail(int code, const char* message) {
  std::cerr << "win32 backdrop erase: " << message << '\n';
  return code;
}

void pump() {
  for (int round = 0; round < 20; ++round) {
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != FALSE) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
    Sleep(5);
  }
}

COLORREF paint_white_and_read(HWND hwnd, bool erase_after) {
  HDC dc = GetDC(hwnd);
  RECT client{};
  GetClientRect(hwnd, &client);
  FillRect(dc, &client, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
  ReleaseDC(hwnd, dc);
  if (erase_after) {
    RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
  }
  pump();
  dc = GetDC(hwnd);
  const COLORREF pixel = GetPixel(dc, 10, 10);
  ReleaseDC(hwnd, dc);
  return pixel;
}

} // namespace

// With the frame extended, DWM composites the GDI redirection surface under
// the transparent swapchain. Left unpainted it is opaque white; GDI black
// (alpha 0) lets the backdrop material show.
int main() {
  auto app = cgpui::create_platform_application();
  if (!app) return fail(1, "platform application was not created");
  cgpui::WindowDescriptor descriptor;
  descriptor.title = "CGPUI backdrop erase test";
  descriptor.size = cgpui::Size{200.0F, 120.0F};
  descriptor.chrome.title_bar = cgpui::WindowTitleBarMode::client;
  auto window = (*app)->create_window(descriptor, [](const cgpui::PlatformEvent&) {});
  if (!window) return fail(2, "window was not created");
  const cgpui::NativeSurfaceHandle native = (*window)->native_surface();
  const auto* surface = std::get_if<cgpui::Win32SurfaceHandle>(&native);
  if (surface == nullptr || surface->hwnd == nullptr) return fail(3, "no HWND");
  auto* hwnd = static_cast<HWND>(surface->hwnd);
  if (GetClassLongPtrW(hwnd, GCLP_HBRBACKGROUND) != 0) {
    return fail(4, "the class keeps no background brush; opaque windows are unchanged");
  }
  if (paint_white_and_read(hwnd, true) != RGB(255, 255, 255)) {
    return fail(5, "without a backdrop WM_ERASEBKGND must not paint");
  }
  const auto state = (*window)->apply_window_backdrop(cgpui::WindowBackdrop::mica, true);
  if (state.applied == cgpui::WindowBackdrop::none) {
    std::cerr << "frame not extended here (" << state.reason << "); skipped\n";
    return 0;
  }
  pump();
  HDC dc = GetDC(hwnd);
  const COLORREF after_apply = GetPixel(dc, 10, 10);
  ReleaseDC(hwnd, dc);
  if (after_apply != RGB(0, 0, 0)) {
    return fail(6, "extending the frame must repaint the redirection surface black");
  }
  if (paint_white_and_read(hwnd, true) != RGB(0, 0, 0)) {
    return fail(7, "WM_ERASEBKGND must fill black while the frame is extended");
  }
  return 0;
}
