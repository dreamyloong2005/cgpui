#include "cgpui/platform/platform.hpp"
#include "../../src/platform/win32_surface_alpha_protocol_internal.hpp"
#include "../../src/platform/win32/win32_window_surface_alpha_internal.hpp"
#include <iostream>

namespace {
int reservation_case(cgpui::WindowTitleBarMode title_bar) {
  using namespace cgpui;
  auto app = create_platform_application();
  if (!app) return 1;
  WindowDescriptor descriptor;
  descriptor.chrome.title_bar = title_bar;
  auto window = (*app)->create_window(descriptor, [](const PlatformEvent&) {});
  if (!window) return 2;
  const auto hwnd = static_cast<HWND>(std::get<Win32SurfaceHandle>((*window)->native_surface()).hwnd);
  const auto instance = static_cast<UINT_PTR>(SendMessageW(hwnd, win32_surface_alpha_instance_message, 0, 0));
  auto send = [&](UINT message, UINT_PTR owner = 19) {
    return static_cast<HRESULT>(SendMessageW(hwnd, message, owner, LPARAM(instance)));
  };
  auto* alpha = win32_window_surface_alpha(hwnd);
  if (!alpha || !instance) return 3;
  const auto capability = send(win32_surface_alpha_query_message, 0);
  if (capability == E_NOTIMPL) {
    if (send(win32_surface_alpha_reserve_message) != E_NOTIMPL) return 4;
    std::cout << "SKIP reservation positive cases: actual OS/topology unsupported\n";
    return 0;
  }
  if (FAILED(capability) || FAILED(send(win32_surface_alpha_reserve_message)) || alpha->active() ||
      FAILED(send(win32_surface_alpha_status_message)) ||
      send(win32_surface_alpha_mark_opaque_message, 20) != HRESULT_FROM_WIN32(ERROR_BUSY) ||
      send(win32_surface_alpha_reserve_message, 20) != HRESULT_FROM_WIN32(ERROR_BUSY)) return 5;
  // Opaque reservation must not impose active-alpha chrome restrictions.
  auto incompatible = descriptor.chrome;
  incompatible.decorations = false;
  if (!alpha->accepts_chrome(incompatible) || !alpha->accepts_native_menu()) return 6;
  (*window)->apply_window_chrome(incompatible);
  if (FAILED(send(win32_surface_alpha_status_message)) ||
      send(win32_surface_alpha_acquire_message) != E_NOTIMPL || alpha->active()) return 7;
  (*window)->apply_window_chrome(descriptor.chrome);
  if (FAILED(send(win32_surface_alpha_acquire_message)) || !alpha->active() ||
      FAILED(send(win32_surface_alpha_suspend_message)) || alpha->active() ||
      FAILED(send(win32_surface_alpha_status_message))) return 8;
  if (FAILED(send(win32_surface_alpha_release_message)) ||
      send(win32_surface_alpha_status_message) != E_INVALIDARG ||
      FAILED(send(win32_surface_alpha_mark_opaque_message)) ||
      send(win32_surface_alpha_reserve_message) != E_NOTIMPL) return 9;
  return 0;
}
} // namespace

int main() {
  for (const auto title_bar : {cgpui::WindowTitleBarMode::system, cgpui::WindowTitleBarMode::client})
    if (const auto result = reservation_case(title_bar)) return result;
  return 0;
}
