#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <thread>
#include <variant>

namespace {

bool near(float lhs, float rhs) {
  return std::fabs(lhs - rhs) < 0.01F;
}

bool same_size(cgpui::Size lhs, cgpui::Size rhs) {
  return near(lhs.width, rhs.width) && near(lhs.height, rhs.height);
}

bool wait_for(const std::atomic_bool& flag) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!flag.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return flag.load();
}

} // namespace

int main() {
  cgpui::test::WaylandTestCompositor compositor("fractional-scale");
  if (!compositor.start()) return 1;
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);
  auto app = cgpui::create_platform_application();
  if (!app) return 2;

  cgpui::PlatformWindow* observed = nullptr;
  std::atomic_bool fractional_matched{false};
  std::atomic_bool resize_matched{false};
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{.size = {320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        const auto* resized = std::get_if<cgpui::WindowResized>(&event);
        if (resized == nullptr || observed == nullptr) return;
        const cgpui::WindowState state = observed->state();
        if (near(resized->scale.value, 1.25F) &&
            same_size(resized->size, {400.0F, 300.0F}) &&
            same_size(state.framebuffer_size, resized->size)) {
          fractional_matched.store(true);
        }
        if (near(resized->scale.value, 1.25F) &&
            same_size(resized->size, {500.0F, 375.0F}) &&
            same_size(state.framebuffer_size, resized->size)) {
          resize_matched.store(true);
          (*app)->quit();
        }
      });
  if (!window) return 3;
  observed = window->get();

  std::thread client([&] { (*app)->run(); });
  const auto finish = [&](int result) {
    (*app)->quit();
    compositor.stop();
    if (client.joinable()) client.join();
    return result;
  };

  compositor.request_fractional_scale(150);
  if (!compositor.wait_for_fractional_scale_sent() ||
      !wait_for(fractional_matched)) {
    return finish(4);
  }
  auto destination = compositor.last_viewport_destination();
  if (compositor.last_surface_buffer_scale() != 2 ||
      destination.width != 320 || destination.height != 240) {
    return finish(5);
  }
  compositor.request_resize_configure(400, 300);
  if (!wait_for(resize_matched)) return finish(6);
  destination = compositor.last_viewport_destination();
  if (destination.width != 400 || destination.height != 300) return finish(7);
  const cgpui::WindowState state = (*window)->state();
  if (!near(state.scale.value, 1.25F) ||
      !same_size(state.framebuffer_size, {500.0F, 375.0F})) {
    return finish(8);
  }
  return finish(0);
}
