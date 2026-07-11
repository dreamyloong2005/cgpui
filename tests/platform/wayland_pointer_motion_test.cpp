#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <variant>
#include <vector>

namespace {

bool wait_for_count(const std::atomic_int& count, int expected) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (count.load() < expected && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return count.load() >= expected;
}

bool same_point(cgpui::Point lhs, cgpui::Point rhs) {
  return std::fabs(lhs.x - rhs.x) < 0.01F &&
      std::fabs(lhs.y - rhs.y) < 0.01F;
}

} // namespace

int main() {
  cgpui::test::WaylandTestCompositor compositor("pointer-motion");
  if (!compositor.start()) {
    return 1;
  }
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 2;
  }

  std::atomic_int move_count{0};
  std::atomic_int exit_count{0};
  std::mutex event_mutex;
  std::vector<cgpui::Point> moves;
  cgpui::Point exit_position{};
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Pointer Motion",
          .size = {320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* moved = std::get_if<cgpui::PointerMoved>(&event)) {
          std::lock_guard lock(event_mutex);
          moves.push_back(moved->position);
          move_count.fetch_add(1);
        } else if (const auto* exited = std::get_if<cgpui::PointerExited>(&event)) {
          std::lock_guard lock(event_mutex);
          exit_position = exited->position;
          exit_count.fetch_add(1);
        } else if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          (*app)->quit();
        }
      });
  if (!window) {
    return 3;
  }

  std::atomic_bool run_finished{false};
  int run_result = -1;
  std::thread client_thread([&] {
    run_result = (*app)->run();
    run_finished.store(true);
  });
  const auto finish = [&](int result) {
    if (!run_finished.load()) {
      (*app)->quit();
    }
    compositor.stop();
    if (client_thread.joinable()) {
      client_thread.join();
    }
    return result;
  };

  compositor.request_pointer_enter(10, 20);
  if (!compositor.wait_for_pointer_enter_sent() ||
      !wait_for_count(move_count, 1)) {
    return finish(4);
  }
  compositor.request_pointer_move(30, 40);
  if (!compositor.wait_for_pointer_move_sent() ||
      !wait_for_count(move_count, 2)) {
    return finish(5);
  }
  compositor.request_pointer_leave();
  if (!compositor.wait_for_pointer_leave_sent() ||
      !wait_for_count(exit_count, 1)) {
    return finish(6);
  }

  compositor.request_close();
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!run_finished.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  if (!run_finished.load() || run_result != 0) {
    return finish(7);
  }
  {
    std::lock_guard lock(event_mutex);
    if (moves.size() != 2 || !same_point(moves[0], {10.0F, 20.0F}) ||
        !same_point(moves[1], {30.0F, 40.0F}) ||
        !same_point(exit_position, {30.0F, 40.0F})) {
      return finish(8);
    }
  }
  return finish(0);
}
