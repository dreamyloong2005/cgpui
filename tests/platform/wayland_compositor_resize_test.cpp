#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <variant>

namespace {

constexpr cgpui::Size requested_size{640.0F, 480.0F};

bool size_equals(cgpui::Size lhs, cgpui::Size rhs) {
  return lhs.width == rhs.width && lhs.height == rhs.height;
}

bool wait_for_run_finished(const std::atomic_bool& run_finished) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!run_finished.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return run_finished.load();
}

} // namespace

int main() {
  cgpui::test::WaylandTestCompositor compositor("resize");
  if (!compositor.start()) {
    return 2;
  }

  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  auto app = cgpui::create_platform_application();
  if (!app) {
    return 3;
  }

  bool resized = false;
  cgpui::Size observed_size{};
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Resize Test",
          .size = cgpui::Size{320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* resize = std::get_if<cgpui::WindowResized>(&event);
            resize != nullptr && size_equals(resize->size, requested_size)) {
          resized = true;
          observed_size = resize->size;
          (*app)->quit();
        } else if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          (*app)->quit();
        }
      });
  if (!window) {
    return 4;
  }

  std::atomic_bool run_finished{false};
  int run_result = -1;
  std::thread client_thread([&] {
    run_result = (*app)->run();
    run_finished.store(true);
  });

  compositor.request_resize_configure(
      static_cast<std::int32_t>(requested_size.width),
      static_cast<std::int32_t>(requested_size.height));

  if (!wait_for_run_finished(run_finished)) {
    compositor.request_close();
    wait_for_run_finished(run_finished);
    compositor.stop();
    if (client_thread.joinable()) {
      client_thread.join();
    }
    return 9;
  }

  if (client_thread.joinable()) {
    client_thread.join();
  }
  compositor.stop();

  if (run_result != 0) {
    return 5;
  }
  if (!compositor.wait_for_resize_configure_sent()) {
    return 6;
  }
  if (!compositor.wait_for_resize_configure_acked()) {
    return 7;
  }
  if (!resized || !size_equals(observed_size, requested_size)) {
    return 8;
  }
  if (!size_equals((*window)->state().framebuffer_size, requested_size)) {
    return 10;
  }

  return 0;
}
