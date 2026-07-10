#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <variant>

namespace {

bool wait_for_value(const std::atomic_int& value, int expected) {
  const auto deadline = std::chrono::steady_clock::now() +
      std::chrono::seconds(3);
  while (value.load() < expected && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return value.load() >= expected;
}

bool wait_for_finished(const std::atomic_bool& finished) {
  const auto deadline = std::chrono::steady_clock::now() +
      std::chrono::seconds(3);
  while (!finished.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return finished.load();
}

} // namespace

int main() {
  cgpui::test::WaylandTestCompositor compositor("display-state");
  if (!compositor.start()) {
    return 1;
  }
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  auto app = cgpui::create_platform_application();
  if (!app) {
    compositor.stop();
    return 2;
  }

  cgpui::PlatformWindow* observed_window = nullptr;
  std::atomic_int observed_stage{0};
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Display State Test",
          .size = cgpui::Size{420.0F, 280.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (observed_window == nullptr ||
            !std::holds_alternative<cgpui::WindowResized>(event)) {
          return;
        }
        const auto display_state =
            observed_window->lifecycle_state().display_state;
        if (observed_stage.load() == 0 &&
            display_state == cgpui::PlatformWindowDisplayState::maximized) {
          observed_stage.store(1);
          (void)observed_window->request_display_state(
              cgpui::PlatformWindowDisplayState::fullscreen);
        } else if (observed_stage.load() == 1 &&
                   display_state ==
                       cgpui::PlatformWindowDisplayState::fullscreen) {
          observed_stage.store(2);
          (void)observed_window->request_display_state(
              cgpui::PlatformWindowDisplayState::normal);
        } else if (observed_stage.load() == 2 &&
                   display_state == cgpui::PlatformWindowDisplayState::normal) {
          observed_stage.store(3);
          (*app)->quit();
        }
      });
  if (!window) {
    compositor.stop();
    return 3;
  }
  observed_window = window->get();

  if (!(*window)->request_display_state(
          cgpui::PlatformWindowDisplayState::minimized) ||
      !compositor.wait_for_minimize_requested() ||
      (*window)->lifecycle_state().display_state !=
          cgpui::PlatformWindowDisplayState::normal) {
    compositor.stop();
    return 4;
  }
  if (!(*window)->request_display_state(
          cgpui::PlatformWindowDisplayState::maximized) ||
      !compositor.wait_for_maximize_requested()) {
    compositor.stop();
    return 5;
  }

  std::atomic_bool finished{false};
  int run_result = -1;
  std::thread client([&] {
    run_result = (*app)->run();
    finished.store(true);
  });
  compositor.request_resize_configure_state(640, 480, true, true, false);
  if (!wait_for_value(observed_stage, 1) ||
      !compositor.wait_for_unmaximize_requested() ||
      !compositor.wait_for_fullscreen_requested()) {
    (*app)->quit();
    compositor.stop();
    client.join();
    return 6;
  }
  compositor.request_resize_configure_state(800, 600, true, false, true);
  if (!wait_for_value(observed_stage, 2) ||
      !compositor.wait_for_unfullscreen_requested()) {
    (*app)->quit();
    compositor.stop();
    client.join();
    return 7;
  }
  compositor.request_resize_configure_state(420, 280, true, false, false);
  if (!wait_for_finished(finished)) {
    (*app)->quit();
    compositor.stop();
    client.join();
    return 8;
  }
  client.join();
  compositor.stop();

  if (run_result != 0 || observed_stage.load() != 3 ||
      (*window)->lifecycle_state().display_state !=
          cgpui::PlatformWindowDisplayState::normal) {
    return 9;
  }
  return 0;
}
