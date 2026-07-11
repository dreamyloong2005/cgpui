#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <wayland-client-protocol.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <string>
#include <thread>
#include <variant>

namespace {

bool wait_for_at_least(const std::atomic_int& value, int expected) {
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::seconds(3);
  while (value.load() < expected &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return value.load() >= expected;
}

} // namespace

int main() {
  cgpui::test::WaylandTestCompositor compositor{"drag-cancel"};
  compositor.set_drag_payloads({cgpui::test::WaylandMimePayload{
      .mime_type = "text/plain",
      .payload = "cancelled drag",
  }});
  compositor.set_drag_source_actions(
      WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY,
      WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
  if (!compositor.start()) return 1;

  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);
  auto app = cgpui::create_platform_application();
  if (!app) return 2;

  std::atomic_int entered{0};
  std::atomic_int exited{0};
  std::atomic_int dropped{0};
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Drag Cancel Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (std::holds_alternative<cgpui::DragEntered>(event)) ++entered;
        if (std::holds_alternative<cgpui::DragExited>(event)) ++exited;
        if (std::holds_alternative<cgpui::DragDropped>(event)) ++dropped;
        if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          (*app)->quit();
        }
      });
  if (!window) return 3;

  std::atomic_bool run_finished{false};
  int run_result = -1;
  std::thread client_thread([&] {
    run_result = (*app)->run();
    run_finished.store(true);
  });

  compositor.request_drag_enter(42, 24);
  if (!compositor.wait_for_drag_enter_sent() || !wait_for_at_least(entered, 1)) {
    compositor.request_close();
  } else {
    compositor.request_drag_leave();
    if (compositor.wait_for_drag_leave_sent() && wait_for_at_least(exited, 1)) {
      compositor.request_drag_leave();
      compositor.request_drag_drop();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    compositor.request_close();
  }

  wait_for_at_least(exited, 1);
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::seconds(3);
  while (!run_finished.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  compositor.stop();
  if (client_thread.joinable()) client_thread.join();

  if (run_result != 0 || entered.load() != 1 || exited.load() != 1 ||
      dropped.load() != 0) return 4;
  if (!compositor.wait_for_drag_offer_destroyed()) return 5;
  if (compositor.drag_offer_finished()) return 6;
  return 0;
}
