#include "cgpui/platform/platform.hpp"
#include "win32_window_client_size_internal.hpp"

#include <cmath>
#include <iostream>
#include <variant>

namespace {

int fail(int code, const char* message) {
  std::cerr << "win32 logical window size: " << message << '\n';
  return code;
}

// WindowDescriptor::size is the logical client size; at 150% a 900x600
// request must produce a 1350x900 physical client area.
int test_conversion_at_synthetic_dpi() {
  const SIZE client = cgpui::win32_physical_client_size({900.0F, 600.0F}, 144);
  if (client.cx != 1350 || client.cy != 900) {
    return fail(1, "logical 900x600 at 144 DPI must be 1350x900 physical");
  }
  const SIZE same = cgpui::win32_physical_client_size({301.0F, 201.0F}, 96);
  if (same.cx != 301 || same.cy != 201) {
    return fail(2, "96 DPI must keep logical pixels unchanged");
  }
  // Frame measured from a real client title bar window at 144 DPI:
  // 11 px side/bottom frame, no caption.
  const SIZE outer = cgpui::win32_outer_size_for_client(
      client, RECT{190, 190, 1090, 790}, RECT{0, 0, 878, 589});
  if (outer.cx != 1372 || outer.cy != 911) {
    return fail(3, "outer size must add the measured non-client frame");
  }
  return 0;
}

int test_created_client_size(cgpui::WindowTitleBarMode mode, int code) {
  auto app = cgpui::create_platform_application();
  if (!app) return fail(code, "platform application was not created");
  cgpui::WindowDescriptor descriptor;
  descriptor.title = "CGPUI logical window size test";
  descriptor.size = cgpui::Size{.width = 300.0F, .height = 200.0F};
  descriptor.chrome.title_bar = mode;
  auto window = (*app)->create_window(descriptor, [](const cgpui::PlatformEvent&) {});
  if (!window) return fail(code + 1, "window was not created");
  const cgpui::NativeSurfaceHandle native = (*window)->native_surface();
  const auto* surface = std::get_if<cgpui::Win32SurfaceHandle>(&native);
  if (surface == nullptr || surface->hwnd == nullptr) return fail(code + 2, "no HWND");
  auto* hwnd = static_cast<HWND>(surface->hwnd);
  const UINT dpi = GetDpiForWindow(hwnd);
  const SIZE expected = cgpui::win32_physical_client_size(descriptor.size, dpi);
  RECT client{};
  GetClientRect(hwnd, &client);
  const cgpui::WindowState state = (*window)->state();
  if (client.right != expected.cx || client.bottom != expected.cy) {
    std::cerr << "dpi " << dpi << " client " << client.right << 'x'
              << client.bottom << " expected " << expected.cx << 'x'
              << expected.cy << '\n';
    return fail(code + 3, "client area must be the logical size at the window DPI");
  }
  if (state.framebuffer_size.width != static_cast<float>(expected.cx) ||
      std::fabs(state.scale.value - static_cast<float>(dpi) / 96.0F) > 0.001F) {
    return fail(code + 4, "window state must report the physical client size");
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_conversion_at_synthetic_dpi(); result != 0) return result;
  if (const int result = test_created_client_size(
          cgpui::WindowTitleBarMode::system, 10); result != 0) {
    return result;
  }
  return test_created_client_size(cgpui::WindowTitleBarMode::client, 20);
}
