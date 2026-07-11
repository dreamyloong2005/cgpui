#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <wayland-client-protocol.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <variant>
#include <vector>

namespace {

bool same_point(cgpui::Point lhs, cgpui::Point rhs) {
  return std::fabs(lhs.x - rhs.x) < 0.01F &&
      std::fabs(lhs.y - rhs.y) < 0.01F;
}

bool wait_for_count(const std::atomic_int& count, int expected) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (count.load() < expected && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return count.load() >= expected;
}

} // namespace

int main() {
  cgpui::test::WaylandTestCompositor compositor("pointer-axis-frame");
  if (!compositor.start()) return 1;
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);
  auto app = cgpui::create_platform_application();
  if (!app) return 2;

  std::atomic_int count{0};
  std::mutex mutex;
  std::vector<cgpui::PointerScrolled> events;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Wayland Axis Frame"},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* scroll = std::get_if<cgpui::PointerScrolled>(&event)) {
          std::lock_guard lock(mutex);
          events.push_back(*scroll);
          count.fetch_add(1);
        }
      });
  if (!window) return 3;

  std::thread client([&] { (*app)->run(); });
  const auto finish = [&](int result) {
    (*app)->quit();
    compositor.stop();
    if (client.joinable()) client.join();
    return result;
  };

  compositor.request_pointer_axis_frame({.delta_x = 8.0F, .delta_y = -16.0F});
  compositor.request_pointer_axis_frame({
      .delta_x = 1.5F,
      .delta_y = -2.5F,
      .source = WL_POINTER_AXIS_SOURCE_WHEEL,
      .value120_x = 30,
      .value120_y = -240,
      .discrete_x = 1,
      .discrete_y = -2,
  });
  compositor.request_pointer_axis_frame({
      .delta_x = 3.0F,
      .delta_y = 4.0F,
      .source = WL_POINTER_AXIS_SOURCE_FINGER,
  });
  compositor.request_pointer_axis_frame({
      .delta_y = 1.0F,
      .source = WL_POINTER_AXIS_SOURCE_WHEEL,
      .value120_y = 120,
  });
  compositor.request_pointer_axis_frame({
      .source = WL_POINTER_AXIS_SOURCE_FINGER,
      .stop_x = true,
      .stop_y = true,
  });
  compositor.request_pointer_axis_frame({.delta_x = 6.0F});
  if (!wait_for_count(count, 5)) return finish(4);

  std::lock_guard lock(mutex);
  if (events.size() != 5) return finish(5);
  if (!same_point(events[0].delta, {8.0F, -16.0F}) || events[0].precise) {
    return finish(6);
  }
  if (!same_point(events[1].delta, {1.5F, -2.5F}) || !events[1].precise) {
    return finish(7);
  }
  if (!same_point(events[2].delta, {3.0F, 4.0F}) || !events[2].precise) {
    return finish(8);
  }
  if (!same_point(events[3].delta, {0.0F, 1.0F}) || events[3].precise) {
    return finish(9);
  }
  if (!same_point(events[4].delta, {6.0F, 0.0F}) || events[4].precise) {
    return finish(10);
  }
  return finish(0);
}
