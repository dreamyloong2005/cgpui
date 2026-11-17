#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

struct wl_client;
struct wl_resource;

namespace cgpui::test {

// Interactive xdg_toplevel requests (move, resize, show_window_menu) and the
// serials of sent pointer button events, recorded by the test compositor.
enum class WaylandToplevelRequestKind { move, resize, show_window_menu };

struct WaylandToplevelRequest {
  WaylandToplevelRequestKind kind = WaylandToplevelRequestKind::move;
  std::uint32_t serial = 0;
  std::uint32_t edges = 0;
  std::int32_t x = 0;
  std::int32_t y = 0;
};

struct WaylandToplevelRequestLog {
  std::mutex mutex;
  std::vector<WaylandToplevelRequest> requests;
  std::vector<std::uint32_t> button_serials;
};

inline WaylandToplevelRequestLog& wayland_toplevel_request_log() {
  static WaylandToplevelRequestLog log;
  return log;
}

inline void wayland_test_record_toplevel_request(WaylandToplevelRequest request) {
  WaylandToplevelRequestLog& log = wayland_toplevel_request_log();
  std::lock_guard lock(log.mutex);
  log.requests.push_back(request);
}

inline void wayland_test_record_pointer_button_serial(std::uint32_t serial) {
  WaylandToplevelRequestLog& log = wayland_toplevel_request_log();
  std::lock_guard lock(log.mutex);
  log.button_serials.push_back(serial);
}

inline void wayland_test_toplevel_show_window_menu(
    wl_client*, wl_resource*, wl_resource*, std::uint32_t serial,
    std::int32_t x, std::int32_t y) {
  wayland_test_record_toplevel_request(
      {.kind = WaylandToplevelRequestKind::show_window_menu, .serial = serial,
       .x = x, .y = y});
}

inline void wayland_test_toplevel_move(
    wl_client*, wl_resource*, wl_resource*, std::uint32_t serial) {
  wayland_test_record_toplevel_request(
      {.kind = WaylandToplevelRequestKind::move, .serial = serial});
}

inline void wayland_test_toplevel_resize(
    wl_client*, wl_resource*, wl_resource*, std::uint32_t serial,
    std::uint32_t edges) {
  wayland_test_record_toplevel_request(
      {.kind = WaylandToplevelRequestKind::resize, .serial = serial,
       .edges = edges});
}

// Waits up to three seconds for `count` requests; returns a snapshot.
inline std::vector<WaylandToplevelRequest> wait_for_toplevel_requests(
    std::size_t count) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  WaylandToplevelRequestLog& log = wayland_toplevel_request_log();
  while (true) {
    {
      std::lock_guard lock(log.mutex);
      if (log.requests.size() >= count ||
          std::chrono::steady_clock::now() >= deadline) {
        return log.requests;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

inline std::vector<std::uint32_t> recorded_pointer_button_serials() {
  WaylandToplevelRequestLog& log = wayland_toplevel_request_log();
  std::lock_guard lock(log.mutex);
  return log.button_serials;
}

} // namespace cgpui::test
