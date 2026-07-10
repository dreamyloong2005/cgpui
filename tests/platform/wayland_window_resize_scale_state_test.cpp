#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <variant>

namespace {

bool size_equal(cgpui::Size lhs, cgpui::Size rhs) {
  return lhs.width == rhs.width && lhs.height == rhs.height;
}

bool wait_for(const std::atomic_bool& flag) {
  const auto deadline = std::chrono::steady_clock::now() +
      std::chrono::seconds(3);
  while (!flag.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return flag.load();
}

} // namespace

int main() {
  cgpui::test::WaylandTestCompositor compositor("resize-scale-state");
  if (!compositor.start()) {
    return 1;
  }
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  auto app = cgpui::create_platform_application();
  if (!app) {
    return 2;
  }

  cgpui::PlatformWindow* observed_window = nullptr;
  std::atomic_bool scale_state_matched{false};
  std::atomic_bool resize_state_matched{false};
  std::atomic_bool finished{false};
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Resize Scale State Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        const auto* resized = std::get_if<cgpui::WindowResized>(&event);
        if (resized == nullptr || observed_window == nullptr) {
          return;
        }
        const cgpui::WindowState state = observed_window->state();
        if (resized->scale.value == 2.0F &&
            size_equal(resized->size, cgpui::Size{640.0F, 480.0F}) &&
            size_equal(state.framebuffer_size, resized->size) &&
            state.scale.value == resized->scale.value) {
          scale_state_matched.store(true);
        }
        if (resized->scale.value == 2.0F &&
            size_equal(resized->size, cgpui::Size{800.0F, 600.0F}) &&
            size_equal(state.framebuffer_size, resized->size) &&
            state.scale.value == resized->scale.value) {
          resize_state_matched.store(true);
          (*app)->quit();
        }
      });
  if (!window) {
    compositor.stop();
    return 3;
  }
  observed_window = window->get();

  int run_result = -1;
  std::thread client([&] {
    run_result = (*app)->run();
    finished.store(true);
  });

  compositor.request_output_scale(2);
  if (!compositor.wait_for_output_scale_sent() || !wait_for(scale_state_matched)) {
    (*app)->quit();
    compositor.stop();
    client.join();
    return 4;
  }
  compositor.request_resize_configure(400, 300);
  if (!wait_for(finished)) {
    (*app)->quit();
    compositor.stop();
    client.join();
    return 5;
  }
  client.join();
  compositor.stop();

  if (run_result != 0 || !resize_state_matched.load()) {
    return 6;
  }
  if (compositor.last_surface_buffer_scale() != 2) {
    return 7;
  }
  const cgpui::WindowState final = (*window)->state();
  if (!size_equal(final.framebuffer_size, cgpui::Size{800.0F, 600.0F}) ||
      final.scale.value != 2.0F) {
    return 8;
  }
  return 0;
}
