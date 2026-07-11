#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>

namespace cgpui::test {

struct WaylandConfigureState {
  std::int32_t width = 0;
  std::int32_t height = 0;
  std::uint32_t serial = 0;
  std::uint32_t acked_serial = 0;
  bool activated = false;
  bool maximized = false;
  bool fullscreen = false;
  bool acked = false;
};

class WaylandTestConfigureState {
 public:
  void request(
      std::int32_t width,
      std::int32_t height,
      bool activated,
      bool maximized,
      bool fullscreen,
      bool include_surface) {
    width_.store(width);
    height_.store(height);
    activated_.store(activated);
    maximized_.store(maximized);
    fullscreen_.store(fullscreen);
    include_surface_.store(include_surface);
    surface_sent_.store(false);
    acked_.store(false);
    serial_.store(0);
    acked_serial_.store(0);
    surface_pending_.store(false);
    toplevel_pending_.store(true);
  }

  void request_surface() {
    surface_sent_.store(false);
    acked_.store(false);
    serial_.store(0);
    acked_serial_.store(0);
    surface_pending_.store(true);
  }

  [[nodiscard]] bool take_toplevel_pending() {
    return toplevel_pending_.exchange(false);
  }
  [[nodiscard]] bool take_surface_pending() {
    return surface_pending_.exchange(false);
  }
  void requeue_toplevel() { toplevel_pending_.store(true); }
  void requeue_surface() { surface_pending_.store(true); }

  [[nodiscard]] bool include_surface() const {
    return include_surface_.load();
  }
  void mark_toplevel_sent() { toplevel_sent_count_.fetch_add(1); }
  void mark_surface_sent(std::uint32_t serial) {
    serial_.store(serial);
    surface_sent_.store(true);
  }
  void acknowledge(std::uint32_t serial) {
    if (serial != serial_.load()) return;
    acked_serial_.store(serial);
    acked_.store(true);
  }

  [[nodiscard]] WaylandConfigureState snapshot() const {
    return WaylandConfigureState{
        .width = width_.load(),
        .height = height_.load(),
        .serial = serial_.load(),
        .acked_serial = acked_serial_.load(),
        .activated = activated_.load(),
        .maximized = maximized_.load(),
        .fullscreen = fullscreen_.load(),
        .acked = acked_.load(),
    };
  }

  [[nodiscard]] bool wait_for_toplevel_sent_count(
      std::uint32_t count) const {
    return wait_for([&] { return toplevel_sent_count_.load() >= count; });
  }
  [[nodiscard]] bool wait_for_surface_sent() const {
    return wait_for([&] { return surface_sent_.load(); });
  }
  [[nodiscard]] bool wait_for_ack() const {
    return wait_for([&] { return acked_.load(); });
  }

 private:
  template <typename Predicate>
  [[nodiscard]] static bool wait_for(Predicate predicate) {
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (!predicate() && std::chrono::steady_clock::now() < deadline) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return predicate();
  }

  std::atomic_bool toplevel_pending_{false};
  std::atomic_bool surface_pending_{false};
  std::atomic_bool include_surface_{false};
  std::atomic_bool surface_sent_{false};
  std::atomic_bool acked_{false};
  std::atomic_bool activated_{false};
  std::atomic_bool maximized_{false};
  std::atomic_bool fullscreen_{false};
  std::atomic_int width_{0};
  std::atomic_int height_{0};
  std::atomic_uint32_t toplevel_sent_count_{0};
  std::atomic_uint32_t serial_{0};
  std::atomic_uint32_t acked_serial_{0};
};

} // namespace cgpui::test
