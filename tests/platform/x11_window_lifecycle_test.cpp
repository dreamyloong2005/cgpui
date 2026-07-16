#include "cgpui/platform/platform.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <variant>

namespace {
bool wait_for(const std::atomic_bool& value) {
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!value.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return value.load();
}
}  // namespace

int main() {
  const char* display = std::getenv("DISPLAY");
  if (display == nullptr || *display == '\0') return 0;
  setenv("CGPUI_LINUX_BACKEND", "x11", 1);
  unsetenv("WAYLAND_DISPLAY");

  auto app = cgpui::create_platform_application();
  if (!app) return 1;
  std::atomic_bool wakeup{false};
  std::atomic_bool close{false};
  cgpui::PlatformWindow* observed = nullptr;
  auto window = (*app)->create_window(
      {.title = "CGPUI X11 Lifecycle", .size = {320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (std::holds_alternative<cgpui::WindowWakeupRequested>(event)) {
          wakeup.store(true);
        }
        if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          close.store(true);
          if (observed != nullptr) {
            observed->resolve_close_request(
                cgpui::PlatformWindowCloseResolution::accept);
          }
        }
      });
  if (!window) return 2;
  observed = window->get();
  const cgpui::NativeSurfaceHandle native_surface = (*window)->native_surface();
  const auto* native =
      std::get_if<cgpui::X11SurfaceHandle>(&native_surface);
  if (native == nullptr || native->display == nullptr || native->window == 0) {
    return 3;
  }
  const auto lifecycle = (*window)->lifecycle_state();
  if (!lifecycle.native_window_created ||
      !lifecycle.initial_configure_complete || lifecycle.close_requested) {
    return 4;
  }

  std::atomic_bool finished{false};
  int run_result = -1;
  std::thread loop([&] {
    run_result = (*app)->run();
    finished.store(true);
  });
  (*app)->request_wakeup();
  if (!wait_for(wakeup)) {
    (*app)->quit();
    loop.join();
    return 5;
  }
  (*window)->request_close();
  if (!wait_for(close)) {
    (*app)->quit();
    loop.join();
    return 6;
  }
  (*app)->quit();
  if (!wait_for(finished)) {
    loop.join();
    return 7;
  }
  loop.join();
  if (run_result != 0 || !(*window)->state().close_requested ||
      !(*window)->close_request_state().accepted) {
    return 8;
  }
  return 0;
}
