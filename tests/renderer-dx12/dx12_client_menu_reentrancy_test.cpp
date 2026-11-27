#include "dx12_client_menu_refusal_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return dx12_fail(1, "menu reentrancy fixture unavailable");
  auto descriptor = desktop.descriptor(true);
  descriptor.chrome.title_bar = WindowTitleBarMode::client;
  auto early = (*app)->create_window(descriptor, [](const PlatformEvent&) {});
  if (!early) return dx12_fail(2, "early client window unavailable");
  if (const auto gated = ordinary_alpha_os_gate(**early)) return *gated;

  enum class Phase { idle, install, clear } phase = Phase::idle;
  PlatformWindow* late_target = nullptr;
  unsigned install_callbacks = 0, clear_callbacks = 0;
  bool inside = false, safe = true;
  RefusalMenuCommands commands;
  auto system = (*app)->create_window(desktop.descriptor(false), [&](const PlatformEvent& event) {
    commands.observe(event);
    if (phase == Phase::idle || inside || !std::holds_alternative<WindowResized>(event)) return;
    inside = true;
    auto& count = phase == Phase::install ? install_callbacks : clear_callbacks;
    ++count;
    // The registry is ordered early-client, system, late-client. During install
    // late has not received its menu; during clear early has already lost it.
    // Thus "there is currently an HMENU" cannot accidentally make this pass.
    PlatformWindow* target = phase == Phase::install ? late_target : early->get();
    if (!target || GetMenu(ordinary_hwnd(*target))) {
      safe = false;
    } else {
      const auto factory = ordinary_renderer(*target, true);
      const bool factory_refused = !factory && factory.error().code == ErrorCode::unsupported_platform;
      safe = safe && factory_refused;
      // Avoid destructive recursive legacy behavior in the baseline RED. Once
      // the factory gate exists, the nested installer must also refuse safely.
      if (factory_refused) {
        auto nested = refusal_native_menu();
        nested.items[0].title = "&Nested";
        nested.items[0].children[0].action_name = "nested.open";
        safe = safe && !(*app)->install_native_menu(std::move(nested)).supported;
      }
      std::cout << "MENU_REENTRANT phase=" << (phase == Phase::install ? "install" : "clear")
                << " callback=" << count << " target-menu=" << GetMenu(ordinary_hwnd(*target))
                << " factory-refused=" << factory_refused << " safe=" << safe << '\n';
    }
    inside = false;
  });
  auto late = (*app)->create_window(descriptor, [](const PlatformEvent&) {});
  if (!system || !late) return dx12_fail(3, "system/late client windows unavailable");
  late_target = late->get();

  phase = Phase::install;
  const auto installed = (*app)->install_native_menu(refusal_native_menu());
  phase = Phase::idle;
  if (!installed.supported || !safe || install_callbacks == 0)
    return dx12_fail(4, "native install had no real reentrant callback or allowed client alpha/nested install");
  const HWND system_hwnd = ordinary_hwnd(**system);
  const HMENU menu = GetMenu(system_hwnd);
  wchar_t title[32]{};
  if (!menu || GetMenu(ordinary_hwnd(**early)) != menu || GetMenu(ordinary_hwnd(**late)) != menu ||
      !GetMenuStringW(menu, 0, title, 32, MF_BYPOSITION) || lstrcmpW(title, L"&File") ||
      !refusal_accelerator(**app, system_hwnd) || commands.accelerator_count != 1 ||
      commands.last_action != "file.open")
    return dx12_fail(5, "nested installation replaced the outer native menu/model/command map");

  phase = Phase::clear;
  const auto cleared = (*app)->install_native_menu({});
  phase = Phase::idle;
  if (!cleared.supported || !safe || clear_callbacks == 0 || GetMenu(system_hwnd) ||
      GetMenu(ordinary_hwnd(**early)) || GetMenu(ordinary_hwnd(**late)))
    return dx12_fail(6, "native clear had no real reentrant callback or allowed client alpha/nested install");
  if (!refusal_accelerator(**app, system_hwnd) || commands.accelerator_count != 1)
    return dx12_fail(7, "native clear left the outer or nested accelerator installed");

  auto renderer = ordinary_renderer(**early, true);
  if (!renderer || !desktop.place(**early) || !refusal_paint(**renderer, **early, false))
    return dx12_fail(8, "completed native installation left the client-alpha transaction gate closed");
  return 0;
}
