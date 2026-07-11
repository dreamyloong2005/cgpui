#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <variant>

namespace {

bool wait_for_value(const std::atomic_int& value, int expected) {
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (value.load() < expected && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return value.load() >= expected;
}

bool wait_for_finished(const std::atomic_bool& finished) {
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!finished.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return finished.load();
}

} // namespace

int main() {
  cgpui::test::WaylandTestCompositor compositor("event-loop-wakeup");
  if (!compositor.start()) return 1;
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  auto app = cgpui::create_platform_application();
  if (!app) return 2;
  std::atomic_int wakeup_count{0};
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Event Loop Wakeup Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (std::holds_alternative<cgpui::WindowWakeupRequested>(event)) {
          wakeup_count.fetch_add(1);
        }
      });
  if (!window) return 3;

  for (int request = 0; request < 32; ++request) (*app)->request_wakeup();
  std::atomic_bool finished{false};
  int run_result = -1;
  std::thread client([&] {
    run_result = (*app)->run();
    finished.store(true);
  });
  const auto finish = [&](int result) {
    if (!finished.load()) (*app)->quit();
    wait_for_finished(finished);
    if (client.joinable()) client.join();
    compositor.stop();
    return result;
  };

  if (!wait_for_value(wakeup_count, 1)) return finish(4);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  if (wakeup_count.load() != 1) return finish(5);

  (*app)->request_wakeup();
  if (!wait_for_value(wakeup_count, 2)) return finish(6);
  (*app)->quit();
  if (!wait_for_finished(finished)) return finish(7);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  if (run_result != 0) return finish(8);
  if (wakeup_count.load() != 2) return finish(9);
  return finish(0);
}
