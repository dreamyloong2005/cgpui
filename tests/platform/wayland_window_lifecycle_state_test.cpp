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
  cgpui::test::WaylandTestCompositor compositor("lifecycle");
  if (!compositor.start()) {
    return 1;
  }
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  auto app = cgpui::create_platform_application();
  if (!app) {
    return 2;
  }

  bool close_event = false;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Lifecycle State Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          close_event = true;
          (*app)->quit();
        }
      });
  if (!window) {
    compositor.stop();
    return 3;
  }

  const cgpui::PlatformWindowLifecycleState initial =
      (*window)->lifecycle_state();
  if (!initial.native_window_created ||
      !initial.initial_configure_complete || initial.close_requested ||
      initial.display_state != cgpui::PlatformWindowDisplayState::normal) {
    compositor.stop();
    return 4;
  }

  std::atomic_bool finished{false};
  int run_result = -1;
  std::thread client([&] {
    run_result = (*app)->run();
    finished.store(true);
  });
  compositor.request_close();

  if (!wait_for_run_finished(finished)) {
    (*app)->quit();
    compositor.stop();
    if (client.joinable()) {
      client.join();
    }
    return 5;
  }
  if (client.joinable()) {
    client.join();
  }
  compositor.stop();

  const cgpui::PlatformWindowLifecycleState closing =
      (*window)->lifecycle_state();
  if (run_result != 0 || !close_event || !closing.close_requested) {
    return 6;
  }
  return 0;
}
