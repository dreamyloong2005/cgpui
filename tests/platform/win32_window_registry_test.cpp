#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <iostream>
#include <memory>
#include <variant>

namespace {

const cgpui::WindowDescriptor descriptor{
    .title = "CGPUI window registry lifetime", .size = {160.0F, 90.0F}};

int destroyed_window_is_unregistered() {
  auto app = cgpui::create_platform_application();
  if (!app) return 1;
  auto window = (*app)->create_window(descriptor, [](const cgpui::PlatformEvent&) {});
  if (!window) return 2;
  window->reset();
  // The original external native-stale probe crashed here: the application
  // dereferenced a window that its caller had already destroyed.
  (void)(*app)->install_native_menu({});
  return 0;
}

int registry_does_not_own_application_or_window() {
  auto app = cgpui::create_platform_application();
  if (!app) return 3;
  auto window = (*app)->create_window(descriptor, [](const cgpui::PlatformEvent&) {});
  if (!window) return 4;
  app->reset(); // A later window destructor must not call a dead application.
  window->reset();
  return 0;
}

int reentrant_wakeup_keeps_surviving_windows() {
  auto app = cgpui::create_platform_application();
  if (!app) return 5;
  std::unique_ptr<cgpui::PlatformWindow> first, removed, survivor, added;
  int first_wakeups = 0, removed_wakeups = 0, survivor_wakeups = 0, added_wakeups = 0;
  int failure = 0;
  const auto first_callback = [&](const cgpui::PlatformEvent& event) {
    if (!std::holds_alternative<cgpui::WindowWakeupRequested>(event)) return;
    ++first_wakeups;
    removed.reset(); // Remove a later entry from the in-progress dispatch.
    auto created = (*app)->create_window(descriptor, [&](const cgpui::PlatformEvent& next) {
      if (std::holds_alternative<cgpui::WindowWakeupRequested>(next)) ++added_wakeups;
    });
    if (!created) { failure = 6; (*app)->quit(); return; }
    added = std::move(*created); // Appending must not invalidate dispatch iterators.
    first.reset(); // The executing callback must survive its owning window.
  };
  auto a = (*app)->create_window(descriptor, first_callback);
  auto b = (*app)->create_window(descriptor, [&](const cgpui::PlatformEvent& event) {
    if (std::holds_alternative<cgpui::WindowWakeupRequested>(event)) ++removed_wakeups;
  });
  auto c = (*app)->create_window(descriptor, [&](const cgpui::PlatformEvent& event) {
    if (!std::holds_alternative<cgpui::WindowWakeupRequested>(event)) return;
    ++survivor_wakeups;
    if (survivor_wakeups == 1) (*app)->request_wakeup();
    else (*app)->quit();
  });
  if (!a || !b || !c) return 7;
  first = std::move(*a); removed = std::move(*b); survivor = std::move(*c);
  (*app)->request_wakeup();
  const int result = (*app)->run();
  // New registrations join the next snapshot, not the dispatch that added them.
  if (result != 0 || failure != 0 || first_wakeups != 1 || removed_wakeups != 0 ||
      survivor_wakeups != 2 || added_wakeups != 1) return 8;
  (void)(*app)->install_native_menu({});
  MSG message{};
  while (PeekMessageW(&message, nullptr, WM_QUIT, WM_QUIT, PM_REMOVE)) {}
  return 0;
}

} // namespace

int main() {
  for (auto test : {destroyed_window_is_unregistered,
                    registry_does_not_own_application_or_window,
                    reentrant_wakeup_keeps_surviving_windows}) {
    if (const int result = test(); result != 0) {
      std::cerr << "Win32 window registry regression: " << result << '\n';
      return result;
    }
  }
  return 0;
}
