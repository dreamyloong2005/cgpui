#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <variant>

namespace {

bool size_equals(cgpui::Size lhs, cgpui::Size rhs) {
  return lhs.width == rhs.width && lhs.height == rhs.height;
}

bool wait_for_count(const std::atomic_int& count, int expected) {
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (count.load() < expected && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return count.load() >= expected;
}

void allow_client_dispatch() {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

} // namespace

int main() {
  constexpr cgpui::Size initial{320.0F, 240.0F};
  constexpr cgpui::Size committed{800.0F, 600.0F};
  cgpui::test::WaylandTestCompositor compositor("configure-lifecycle");
  if (!compositor.start()) return 1;
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  auto app = cgpui::create_platform_application();
  if (!app) return 2;

  std::atomic_int resize_count{0};
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Configure Lifecycle Test",
          .size = initial},
      [&](const cgpui::PlatformEvent& event) {
        if (std::holds_alternative<cgpui::WindowResized>(event)) {
          resize_count.fetch_add(1);
        } else if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          (*app)->quit();
        }
      });
  if (!window || resize_count.load() != 1) return 3;

  std::atomic_bool finished{false};
  std::thread client([&] {
    (void)(*app)->run();
    finished.store(true);
  });
  const auto finish = [&](int result) {
    if (!finished.load()) compositor.request_close();
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (!finished.load() && std::chrono::steady_clock::now() < deadline) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!finished.load()) (*app)->quit();
    if (client.joinable()) client.join();
    compositor.stop();
    return finished.load() ? result : 16;
  };

  compositor.request_toplevel_configure_state(640, 480, true, false, false);
  if (!compositor.wait_for_toplevel_configure_sent_count(1)) return finish(4);
  allow_client_dispatch();
  if (!size_equals((*window)->state().framebuffer_size, initial) ||
      resize_count.load() != 1) return finish(5);

  compositor.request_toplevel_configure_state(0, 0, false, false, false);
  if (!compositor.wait_for_toplevel_configure_sent_count(2)) return finish(6);
  compositor.request_surface_configure();
  if (!compositor.wait_for_resize_configure_acked()) return finish(7);
  allow_client_dispatch();
  if (!size_equals((*window)->state().framebuffer_size, initial) ||
      resize_count.load() != 1) return finish(8);

  compositor.request_toplevel_configure_state(
      static_cast<std::int32_t>(committed.width),
      static_cast<std::int32_t>(committed.height),
      true,
      false,
      false);
  if (!compositor.wait_for_toplevel_configure_sent_count(3)) return finish(9);
  allow_client_dispatch();
  if (!size_equals((*window)->state().framebuffer_size, initial)) {
    return finish(10);
  }
  compositor.request_surface_configure();
  if (!compositor.wait_for_resize_configure_acked() ||
      !wait_for_count(resize_count, 2)) return finish(11);
  if (!size_equals((*window)->state().framebuffer_size, committed)) {
    return finish(12);
  }

  compositor.request_toplevel_configure_state(800, 600, true, false, false);
  if (!compositor.wait_for_toplevel_configure_sent_count(4)) return finish(13);
  compositor.request_surface_configure();
  if (!compositor.wait_for_resize_configure_acked()) return finish(14);
  allow_client_dispatch();
  if (resize_count.load() != 2) return finish(15);
  return finish(0);
}
