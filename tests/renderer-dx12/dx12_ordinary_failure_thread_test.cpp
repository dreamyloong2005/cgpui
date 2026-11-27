#include "dx12_ordinary_pixels_support.hpp"
#include "dx12_ordinary_menu_pixels_support.hpp"
#include <thread>
#include <string>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return dx12_fail(1, "thread/failure fixture unavailable");
  auto window = (*app)->create_window(desktop.descriptor(true), [](const PlatformEvent&) {});
  if (!window || !ordinary_install_menu(**app, **window) || !desktop.place(**window))
    return dx12_fail(2, "thread/failure ordinary window unavailable");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  MenuPixels reference;
  MenuFocus focus;
  if (!stable_menu(ordinary_hwnd(**window), reference, focus)) return dx12_fail(3, "menu reference unstable");
  const char* previous = std::getenv("CGPUI_DX12_ADAPTER");
  const std::string selection = previous ? previous : "";
  _putenv_s("CGPUI_DX12_ADAPTER", "999999");
  auto failed = ordinary_renderer(**window, true); // Lease acquired; native device selection then fails.
  _putenv_s("CGPUI_DX12_ADAPTER", selection.c_str());
  if (failed) return dx12_fail(4, "invalid adapter unexpectedly succeeded");
  MenuPixels restored;
  MenuFocus restored_focus;
  if (!stable_menu(ordinary_hwnd(**window), restored, restored_focus) ||
      reference.different(restored) || focus != restored_focus)
    return dx12_fail(5, "failed factory damaged native menu or focus");
  auto renderer = ordinary_renderer(**window, true);
  if (!renderer || !ordinary_draw_bands(**renderer, **window, false))
    return dx12_fail(6, "partial initialization did not release the alpha lease");
  bool refused = false;
  std::thread caller([&] {
    auto frame = (*renderer)->begin_frame();
    auto resize = (*renderer)->resize({64, 64}, {1});
    auto change = (*renderer)->set_transparent_surface(false);
    refused = !frame && frame.error().code == ErrorCode::invalid_argument &&
        !resize && resize.error().code == ErrorCode::invalid_argument &&
        !change && change.error().code == ErrorCode::invalid_argument;
  });
  caller.join();
  if (!refused || !ordinary_draw_bands(**renderer, **window, true) ||
      !ordinary_desktop_bands(**window, true, true))
    return dx12_fail(7, "foreign-thread refusal changed live ordinary alpha rendering");
  std::thread destructor([owned = std::move(*renderer)]() mutable { owned.reset(); });
  destructor.join(); // No UI SendMessage deadlock: lease release carries only queued tokens.
  pump_messages_for(100);
  auto replacement = ordinary_renderer(**window, true);
  if (!replacement || !ordinary_draw_bands(**replacement, **window, false) ||
      !ordinary_desktop_bands(**window, false, true))
    return dx12_fail(8, "foreign-thread destruction did not retire its window lease");
  return 0;
}
