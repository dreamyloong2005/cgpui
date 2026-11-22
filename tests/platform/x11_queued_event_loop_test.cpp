#include "cgpui/platform/platform.hpp"

#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <poll.h>
#include <thread>
#include <variant>
#include <xcb/xcb.h>

namespace {
bool queue_motion(xcb_connection_t* connection, xcb_window_t window) {
  xcb_motion_notify_event_t event{};
  event.response_type = XCB_MOTION_NOTIFY;
  event.event = window;
  event.event_x = -1234;
  event.event_y = -2345;
  // A zero mask delivers directly to the owner, even for an unmapped window.
  xcb_send_event(connection, false, window, XCB_EVENT_MASK_NO_EVENT,
                 reinterpret_cast<const char*>(&event));
  // The reply comes after the event on this connection. Reading it queues the
  // event inside XCB, rather than leaving bytes readable on the socket.
  auto* reply = xcb_get_input_focus_reply(
      connection, xcb_get_input_focus(connection), nullptr);
  const bool replied = reply != nullptr;
  std::free(reply);
  pollfd socket{.fd = xcb_get_file_descriptor(connection), .events = POLLIN};
  return replied && poll(&socket, 1, 0) == 0;
}

int test_queued_event(bool from_wakeup) {
  auto app = cgpui::create_platform_application();
  if (!app) {
    std::cerr << "queued X11 event: from_wakeup=" << from_wakeup
              << " application: " << app.error().message << '\n';
    return 1;
  }
  xcb_connection_t* connection = nullptr;
  xcb_window_t window_id = XCB_WINDOW_NONE;
  int matched = 0;
  bool queued = !from_wakeup;
  auto window = (*app)->create_window(
      {.title = "CGPUI queued X11 events", .size = {32.0F, 32.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (from_wakeup &&
            std::holds_alternative<cgpui::WindowWakeupRequested>(event)) {
          queued = queue_motion(connection, window_id);
          if (!queued) (*app)->quit();
        } else if (const auto* motion = std::get_if<cgpui::PointerMoved>(&event);
                   motion && motion->position.x == -1234.0F &&
                   motion->position.y == -2345.0F) {
          ++matched;
          (*app)->quit();
        }
      });
  if (!window) {
    std::cerr << "queued X11 event: from_wakeup=" << from_wakeup
              << " window: " << window.error().message << '\n';
    return 2;
  }
  const auto native = (*window)->native_surface();
  const auto* surface = std::get_if<cgpui::X11SurfaceHandle>(&native);
  if (surface == nullptr) return 3;
  connection = static_cast<xcb_connection_t*>(surface->display);
  window_id = static_cast<xcb_window_t>(surface->window);
  // Do not rely on a later real Expose or pointer event to wake the socket.
  const std::uint32_t event_mask = XCB_EVENT_MASK_NO_EVENT;
  xcb_change_window_attributes(connection, window_id, XCB_CW_EVENT_MASK, &event_mask);
  xcb_unmap_window(connection, window_id);
  if (from_wakeup) {
    (*app)->request_wakeup();
  } else if (!queue_motion(connection, window_id)) {
    std::cerr << "queued X11 event: initial socket was not drained or reply failed\n";
    return 4;
  }

  std::mutex mutex;
  std::condition_variable completed;
  bool finished = false, timed_out = false;
  std::thread watchdog([&] {
    std::unique_lock lock(mutex);
    if (!completed.wait_for(lock, std::chrono::seconds(3), [&] { return finished; })) {
      timed_out = true;
      (*app)->quit();
    }
  });
  const int result = (*app)->run();
  {
    std::lock_guard lock(mutex);
    finished = true;
  }
  completed.notify_one();
  watchdog.join();
  if (result != 0 || timed_out || !queued || matched != 1) {
    std::cerr << "queued X11 event: from_wakeup=" << from_wakeup
              << " result=" << result << " timeout=" << timed_out
              << " queued=" << queued << " matched=" << matched << '\n';
    return 5;
  }
  return 0;
}
} // namespace

int main() {
  const char* display = std::getenv("DISPLAY");
  if (display == nullptr || *display == '\0') return 0;
  setenv("CGPUI_LINUX_BACKEND", "x11", 1);
  unsetenv("WAYLAND_DISPLAY");
  const int initial = test_queued_event(false);
  const int wakeup = test_queued_event(true);
  return initial != 0 ? initial : wakeup;
}
