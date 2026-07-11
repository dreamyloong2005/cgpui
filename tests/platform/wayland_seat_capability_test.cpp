#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <wayland-client-protocol.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <variant>

namespace {

bool wait_for_count(const std::atomic_int& count, int expected) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (count.load() < expected && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return count.load() >= expected;
}

bool wait_for_finished(const std::atomic_bool& finished) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!finished.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return finished.load();
}

} // namespace

int main() {
  cgpui::test::WaylandTestCompositor compositor("seat-capability");
  if (!compositor.start()) {
    return 1;
  }
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  auto app = cgpui::create_platform_application();
  if (!app) {
    return 2;
  }

  std::atomic_int focus_count{0};
  std::atomic_int blur_count{0};
  std::atomic_int pointer_count{0};
  std::atomic_int key_count{0};
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Seat Capability",
          .size = {320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* focus = std::get_if<cgpui::WindowFocused>(&event)) {
          (focus->focused ? focus_count : blur_count).fetch_add(1);
        } else if (std::holds_alternative<cgpui::PointerMoved>(event)) {
          pointer_count.fetch_add(1);
        } else if (std::holds_alternative<cgpui::KeyboardKey>(event)) {
          key_count.fetch_add(1);
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
      wait_for_finished(run_finished);
    }
    compositor.stop();
    if (client_thread.joinable()) {
      client_thread.join();
    }
    return result;
  };

  if (!compositor.wait_for_pointer_bound(true) ||
      !compositor.wait_for_keyboard_bound(true)) {
    return finish(4);
  }
  compositor.request_pointer_move(20, 30);
  compositor.request_keyboard_key(30, true);
  compositor.request_keyboard_key(30, false);
  if (!wait_for_count(focus_count, 1) || !wait_for_count(pointer_count, 1) ||
      !wait_for_count(key_count, 2)) {
    return finish(5);
  }

  compositor.request_seat_capabilities(WL_SEAT_CAPABILITY_KEYBOARD);
  if (!compositor.wait_for_pointer_bound(false)) {
    return finish(6);
  }
  if (!compositor.wait_for_keyboard_bound(true)) {
    return finish(7);
  }
  if (blur_count.load() != 0) {
    return finish(8);
  }

  compositor.request_seat_capabilities(0);
  if (!compositor.wait_for_keyboard_bound(false)) {
    return finish(9);
  }
  if (!wait_for_count(blur_count, 1)) {
    return finish(10);
  }

  compositor.request_seat_capabilities(WL_SEAT_CAPABILITY_POINTER);
  if (!compositor.wait_for_pointer_bound(true) ||
      !compositor.wait_for_keyboard_bound(false)) {
    return finish(11);
  }
  compositor.request_pointer_move(40, 50);
  if (!wait_for_count(pointer_count, 2) || focus_count.load() != 1) {
    return finish(11);
  }

  compositor.request_seat_capabilities(
      WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD);
  if (!compositor.wait_for_pointer_bound(true) ||
      !compositor.wait_for_keyboard_bound(true)) {
    return finish(13);
  }
  compositor.request_keyboard_key(30, true);
  compositor.request_keyboard_key(30, false);
  if (!wait_for_count(focus_count, 2) || !wait_for_count(key_count, 4)) {
    return finish(14);
  }

  compositor.request_close();
  wait_for_finished(run_finished);
  return finish(run_finished.load() && run_result == 0 ? 0 : 15);
}
