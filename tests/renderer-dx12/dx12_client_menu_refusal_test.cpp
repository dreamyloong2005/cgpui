#include "dx12_client_menu_refusal_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return dx12_fail(1, "client-menu refusal fixture unavailable");
  RefusalMenuCommands commands;
  auto descriptor = desktop.descriptor(true);
  descriptor.chrome.title_bar = WindowTitleBarMode::client;
  auto window = (*app)->create_window(descriptor,
      [&](const PlatformEvent& event) { commands.observe(event); });
  if (!window || !desktop.place(**window)) return dx12_fail(2, "initial client window unavailable");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  const HWND hwnd = ordinary_hwnd(**window);

  // Existing native menus are kept, never hidden or converted into an overlay.
  const auto installed = (*app)->install_native_menu(refusal_native_menu());
  if (!installed.supported || installed.registered_accelerator_count != 1 || !GetMenu(hwnd))
    return dx12_fail(3, "pre-renderer native menu unavailable");
  if (!client_menu_alpha_refused(**window))
    return dx12_fail(4, "menu-bearing client alpha creation accepted or changed the native menu");
  if (!refusal_accelerator(**app, hwnd) || commands.accelerator_count != 1 || commands.last_action != "file.open")
    return dx12_fail(5, "factory refusal altered the existing native accelerator");
  if (!(*app)->install_native_menu({}).supported || GetMenu(hwnd))
    return dx12_fail(6, "native menu clear failed");

  auto renderer = ordinary_renderer(**window, true);
  if (!renderer || !refusal_paint(**renderer, **window, false))
    return dx12_fail(7, "menu-free client alpha unavailable after menu removal");
  RefusalNativeSnapshot active;
  if (!active.capture(**window)) return dx12_fail(8, "active client snapshot failed");
  const auto refused = (*app)->install_native_menu(refusal_native_menu());
  if (refused.supported || !active.same(**window) || GetMenu(hwnd) ||
      !refusal_paint(**renderer, **window, true))
    return dx12_fail(9, "active client-alpha menu installation accepted, mutated topology or broke paint");
  if (!refusal_accelerator(**app, hwnd) || commands.accelerator_count != 1)
    return dx12_fail(10, "rejected native menu registered its accelerator");

  // A later public window must inherit the unchanged empty application model.
  auto later = (*app)->create_window(desktop.descriptor(false), [](const PlatformEvent&) {});
  if (!later || GetMenu(ordinary_hwnd(**later)) || GetMenu(hwnd))
    return dx12_fail(11, "rejected menu mutated the application model or another window");
  later->reset();
  if (!desktop.place(**window) || !(*app)->install_native_menu({}).supported ||
      !active.same(**window) || !refusal_paint(**renderer, **window, false))
    return dx12_fail(12, "empty menu installation must remain usable during client alpha");

  // The last owner releases the restriction; a new menu then blocks alpha again.
  renderer->reset();
  const auto after_release = (*app)->install_native_menu(refusal_native_menu());
  if (!after_release.supported || !GetMenu(hwnd) || !client_menu_alpha_refused(**window))
    return dx12_fail(13, "released client lease still blocks native menu or permits menu-bearing alpha");
  if (!refusal_accelerator(**app, hwnd) || commands.accelerator_count != 2 || commands.last_action != "file.open")
    return dx12_fail(14, "post-release native menu accelerator is unusable");
  window->reset();
  if (!(*app)->install_native_menu({}).supported) return dx12_fail(15, "system control menu clear failed");

  // System-caption alpha remains compatible with real native menus, including
  // installation after alpha creation. This guard must not become global.
  auto system = (*app)->create_window(desktop.descriptor(true), [](const PlatformEvent&) {});
  if (!system || !desktop.place(**system)) return dx12_fail(16, "system-caption control unavailable");
  auto system_renderer = ordinary_renderer(**system, true);
  if (!system_renderer || !refusal_paint(**system_renderer, **system, false))
    return dx12_fail(17, "system-caption initial alpha unavailable");
  if (!(*app)->install_native_menu(refusal_native_menu()).supported || !GetMenu(ordinary_hwnd(**system)) ||
      !refusal_paint(**system_renderer, **system, true))
    return dx12_fail(18, "client refusal leaked into system-caption native menu support");
  return 0;
}
