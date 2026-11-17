#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"
#include "wayland_test_toplevel_requests.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <variant>

namespace {

using cgpui::Rect;
using cgpui::WindowControlArea;
using cgpui::test::WaylandToplevelRequestKind;

constexpr std::uint32_t left_button = 0x110;
constexpr std::uint32_t right_button = 0x111;
constexpr std::uint32_t resize_edge_left = 4;

int fail(int code, const char* message) {
  std::cerr << "wayland client title bar: " << message << '\n';
  return code;
}

bool wait_for_serials(std::size_t count) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (cgpui::test::recorded_pointer_button_serials().size() < count) {
    if (std::chrono::steady_clock::now() >= deadline) return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  return true;
}

void click(cgpui::test::WaylandTestCompositor& compositor, std::uint32_t button) {
  compositor.request_pointer_button(button, true);
  compositor.request_pointer_button(button, false);
}

// The compositor sends motion before buttons within one loop iteration but
// has no motion counter; give the motion an iteration of its own.
void move_to(cgpui::test::WaylandTestCompositor& compositor, int x, int y) {
  compositor.request_pointer_move(x, y);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

} // namespace

int main() {
  cgpui::test::WaylandTestCompositor compositor("client-title-bar");
  if (!compositor.start()) return fail(1, "compositor did not start");
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);
  auto app = cgpui::create_platform_application();
  if (!app) return fail(2, "platform application was not created");
  cgpui::WindowDescriptor descriptor{
      .title = "CGPUI Wayland Client Title Bar Test", .size = {320.0F, 240.0F}};
  descriptor.chrome.title_bar = cgpui::WindowTitleBarMode::client;
  std::atomic_bool close_requested{false};
  auto window = (*app)->create_window(descriptor, [&](const cgpui::PlatformEvent& event) {
    if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
      close_requested.store(true);
      (*app)->quit();
    }
  });
  if (!window) return fail(3, "window was not created");
  if (!compositor.wait_for_client_side_decoration_requested()) {
    return fail(4, "client mode must request client-side decorations");
  }
  const cgpui::PlatformWindowFrameMetrics metrics = (*window)->frame_metrics();
  std::cout << "metrics: resize_border " << metrics.resize_border << ", mechanism '"
            << metrics.mechanism << "'\n";
  if (!metrics.supported || !metrics.client_title_bar ||
      metrics.resize_border != 6.0F || metrics.client_resize_border.left != 6.0F ||
      metrics.caption_height != 0.0F || metrics.reason.empty()) {
    return fail(5, "frame metrics must describe the in-surface resize band");
  }
  const cgpui::PlatformWindowActionResult early = (*window)->perform_window_action(
      {.action = cgpui::PlatformWindowAction::begin_move});
  if (early.supported || early.reason.empty()) {
    return fail(6, "begin_move without a button serial must be unsupported");
  }
  (*window)->set_window_control_regions({
      {WindowControlArea::caption, Rect{{0, 0}, {320, 32}}},
      {WindowControlArea::client, Rect{{100, 4}, {60, 24}}},
      {WindowControlArea::minimize_button, Rect{{200, 0}, {40, 32}}},
      {WindowControlArea::maximize_button, Rect{{240, 0}, {40, 32}}},
      {WindowControlArea::close_button, Rect{{280, 0}, {40, 32}}},
  });

  int run_result = -1;
  std::atomic_bool run_finished{false};
  std::thread client([&] {
    run_result = (*app)->run();
    run_finished.store(true);
  });
  const auto finish = [&](int code, const char* message) {
    if (!run_finished.load()) compositor.request_close();
    if (client.joinable()) client.join();
    compositor.stop();
    return code == 0 ? 0 : fail(code, message);
  };

  compositor.request_pointer_enter(50, 16);
  if (!compositor.wait_for_pointer_enter_sent()) return finish(10, "no pointer enter");
  click(compositor, left_button);  // caption press: move
  click(compositor, left_button);  // second press within 400 ms: maximize
  if (!wait_for_serials(4) || !compositor.wait_for_maximize_requested()) {
    return finish(11, "double click on the caption must maximize");
  }
  move_to(compositor, 2, 120);
  if (!compositor.wait_for_pointer_move_sent()) return finish(12, "no pointer move");
  click(compositor, left_button);  // left edge: resize
  if (!wait_for_serials(6)) return finish(13, "edge click not sent");
  move_to(compositor, 50, 16);
  click(compositor, right_button);  // caption right press: window menu
  if (!wait_for_serials(8)) return finish(14, "right click not sent");
  move_to(compositor, 120, 16);
  click(compositor, left_button);  // client control inside the caption: nothing
  if (!wait_for_serials(10)) return finish(15, "client click not sent");
  move_to(compositor, 220, 16);
  click(compositor, left_button);  // minimize button
  if (!wait_for_serials(12) || !compositor.wait_for_minimize_requested()) {
    return finish(16, "releasing over the minimize button must minimize");
  }
  move_to(compositor, 300, 16);
  click(compositor, left_button);  // close button
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!run_finished.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  if (!close_requested.load()) return finish(17, "the close button must request close");

  const auto requests = cgpui::test::wait_for_toplevel_requests(3);
  const auto serials = cgpui::test::recorded_pointer_button_serials();
  for (const auto& request : requests) {
    std::cout << "request " << static_cast<int>(request.kind) << " serial "
              << request.serial << " edges " << request.edges << " at "
              << request.x << ',' << request.y << '\n';
  }
  if (requests.size() != 3 || serials.size() < 8 ||
      requests[0].kind != WaylandToplevelRequestKind::move ||
      requests[0].serial != serials[0] ||
      requests[1].kind != WaylandToplevelRequestKind::resize ||
      requests[1].serial != serials[4] || requests[1].edges != resize_edge_left ||
      requests[2].kind != WaylandToplevelRequestKind::show_window_menu ||
      requests[2].serial != serials[6] || requests[2].x != 50 || requests[2].y != 16) {
    return finish(18, "move, resize and window menu must carry the press serial");
  }
  return finish(0, "");
}
