#include "dx12_dynamic_client_test_support.hpp"
#include "dx12_client_alpha_test_support.hpp"
#include "dx12_client_menu_refusal_support.hpp"

int main() {
  using namespace cgpui;
  using namespace cgpui_test;
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return 1;
  auto descriptor = client_alpha_descriptor(desktop);
  descriptor.chrome.transparent_background = false;
  auto early = (*app)->create_window(descriptor, [](const PlatformEvent&) {});
  if (!early) return 2;
  if (const auto gated = ordinary_alpha_os_gate(**early)) return *gated;
  auto early_renderer = ordinary_renderer(**early, false);
  if (!early_renderer) return 3;
  enum class Phase { idle, install, clear } phase = Phase::idle;
  PlatformWindow* late_window = nullptr;
  Renderer* late_renderer = nullptr;
  bool inside = false, safe = true;
  unsigned installed_callbacks = 0, cleared_callbacks = 0;
  auto system = (*app)->create_window(desktop.descriptor(false), [&](const PlatformEvent& event) {
    if (inside || phase == Phase::idle || !std::holds_alternative<WindowResized>(event)) return;
    inside = true;
    const bool installing = phase == Phase::install;
    ++(installing ? installed_callbacks : cleared_callbacks);
    auto* target = installing ? late_window : early->get();
    auto* target_renderer = installing ? late_renderer : early_renderer->get();
    // In ordered registry callbacks, the selected client currently has no menu.
    // Only supported()'s in-flight installation gate can reject the promotion.
    if (!target || !target_renderer || GetMenu(ordinary_hwnd(*target))) safe = false;
    else {
      RefusalNativeSnapshot before;
      if (!before.capture(*target)) safe = false;
      const auto promoted = target_renderer->set_transparent_surface(true);
      safe = safe && !promoted && promoted.error().code == ErrorCode::unsupported_platform &&
          before.same(*target) && ordinary_composition(target_renderer->surface_composition(), false);
    }
    inside = false;
  });
  auto late = (*app)->create_window(descriptor, [](const PlatformEvent&) {});
  if (!system || !late) return 4;
  auto late_owner = ordinary_renderer(**late, false);
  if (!late_owner) return 5;
  late_window = late->get(); late_renderer = late_owner->get();
  phase = Phase::install;
  const auto installed = (*app)->install_native_menu(refusal_native_menu());
  phase = Phase::idle;
  if (!installed.supported || !safe || !installed_callbacks) return 6;
  phase = Phase::clear;
  const auto cleared = (*app)->install_native_menu({});
  phase = Phase::idle;
  if (!cleared.supported || !safe || !cleared_callbacks || GetMenu(ordinary_hwnd(**early)) ||
      GetMenu(ordinary_hwnd(**late))) return 7;
  // The failed callbacks did not poison either reservation or invent flip history.
  if (!desktop.place(**early) || !dynamic_client_resize(**early_renderer, **early) ||
      !(*early_renderer)->set_transparent_surface(true) || !dynamic_client_paint(**early_renderer, **early, false, true)) return 8;
  return 0;
}
