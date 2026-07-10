#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <variant>

namespace {

bool wait_for_run_finished(const std::atomic_bool& finished) {
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::seconds(3);
  while (!finished.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return finished.load();
}

} // namespace

int main() {
  cgpui::test::WaylandTestCompositor compositor("focus-state");
  if (!compositor.start()) {
    return 1;
  }
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  auto app = cgpui::create_platform_application();
  if (!app) {
    return 2;
  }

  cgpui::PlatformWindow* observed_window = nullptr;
  bool activated = false;
  bool deactivated = false;
  bool focused = false;
  bool blurred = false;
  bool activation_state_matched = false;
  bool deactivation_state_matched = false;
  bool focus_state_matched = false;
  bool blur_state_matched = false;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Activation Focus State Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* activation = std::get_if<cgpui::WindowActivated>(&event);
            activation != nullptr && observed_window != nullptr) {
          const auto lifecycle = observed_window->lifecycle_state();
          activated = activated || activation->active;
          deactivated = deactivated || !activation->active;
          activation_state_matched =
              activation_state_matched ||
              (activation->active && lifecycle.active);
          deactivation_state_matched =
              deactivation_state_matched ||
              (!activation->active && !lifecycle.active);
        }
        if (const auto* focus = std::get_if<cgpui::WindowFocused>(&event);
            focus != nullptr && observed_window != nullptr) {
          const auto lifecycle = observed_window->lifecycle_state();
          focused = focused || focus->focused;
          blurred = blurred || !focus->focused;
          focus_state_matched =
              focus_state_matched || (focus->focused && lifecycle.focused);
          blur_state_matched =
              blur_state_matched || (!focus->focused && !lifecycle.focused);
        }
        if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          (*app)->quit();
        }
      });
  if (!window) {
    compositor.stop();
    return 3;
  }
  observed_window = window->get();

  std::atomic_bool finished{false};
  int run_result = -1;
  std::thread client([&] {
    run_result = (*app)->run();
    finished.store(true);
  });

  compositor.request_resize_configure_state(320, 240, true, false, false);
  if (!compositor.wait_for_resize_configure_acked()) {
    return 4;
  }
  compositor.request_keyboard_key(30, true);
  if (!compositor.wait_for_keyboard_key_sent()) {
    return 5;
  }
  compositor.request_keyboard_leave();
  if (!compositor.wait_for_keyboard_leave_sent()) {
    return 6;
  }
  compositor.request_resize_configure_state(320, 240, false, false, false);
  if (!compositor.wait_for_resize_configure_acked()) {
    return 7;
  }
  compositor.request_close();

  if (!wait_for_run_finished(finished)) {
    (*app)->quit();
    compositor.stop();
    if (client.joinable()) {
      client.join();
    }
    return 8;
  }
  if (client.joinable()) {
    client.join();
  }
  compositor.stop();

  if (run_result != 0 || !activated || !deactivated || !focused || !blurred) {
    return 9;
  }
  if (!activation_state_matched || !deactivation_state_matched ||
      !focus_state_matched || !blur_state_matched) {
    return 10;
  }
  const auto final = (*window)->lifecycle_state();
  if (final.active || final.focused) {
    return 11;
  }
  return 0;
}
