#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <variant>

namespace {

constexpr std::uint32_t expected_key = 30;

bool wait_for_run_finished(const std::atomic_bool& run_finished) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!run_finished.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return run_finished.load();
}

} // namespace

int main() {
  cgpui::test::WaylandTestCompositor compositor("keyboard");
  if (!compositor.start()) {
    return 2;
  }

  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  auto app = cgpui::create_platform_application();
  if (!app) {
    return 3;
  }

  bool pressed = false;
  bool released = false;
  bool focused = false;
  bool blurred = false;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Keyboard Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* focus = std::get_if<cgpui::WindowFocused>(&event);
            focus != nullptr) {
          focused = focused || focus->focused;
          blurred = blurred || !focus->focused;
        }
        if (const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
            key != nullptr && key->key_code == expected_key) {
          pressed = pressed || key->action == cgpui::KeyAction::pressed;
          released = released || key->action == cgpui::KeyAction::released;
        }
        if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          (*app)->quit();
        }
        if (focused && pressed && released && blurred) {
          (*app)->quit();
        }
      });
  if (!window) {
    return 4;
  }

  std::atomic_bool run_finished{false};
  int run_result = -1;
  std::thread client_thread([&] {
    run_result = (*app)->run();
    run_finished.store(true);
  });

  compositor.request_keyboard_key(expected_key, true);
  compositor.request_keyboard_key(expected_key, false);
  compositor.request_keyboard_leave();

  if (!wait_for_run_finished(run_finished)) {
    compositor.request_close();
    wait_for_run_finished(run_finished);
    compositor.stop();
    if (client_thread.joinable()) {
      client_thread.join();
    }
    return 9;
  }

  if (client_thread.joinable()) {
    client_thread.join();
  }
  compositor.stop();

  if (run_result != 0) {
    return 5;
  }
  if (!compositor.wait_for_keyboard_key_sent()) {
    return 6;
  }
  if (!pressed || !released) {
    return 7;
  }
  if (!focused) {
    return 8;
  }
  if (!blurred) {
    return 10;
  }

  return 0;
}
