#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>

namespace cgpui::test {

struct WaylandCursorSurfaceState {
  std::uint32_t apply_count = 0;
  std::int32_t hotspot_x = 0;
  std::int32_t hotspot_y = 0;
  std::int32_t buffer_scale = 1;
  bool surface_set = false;
  bool buffer_attached = false;
  bool surface_committed = false;
};

class WaylandTestCursorState {
 public:
  void record_set_cursor(bool surface_set, std::int32_t x, std::int32_t y) {
    surface_set_.store(surface_set);
    hotspot_x_.store(x);
    hotspot_y_.store(y);
    apply_count_.fetch_add(1);
  }
  void record_buffer_attached() { buffer_attached_.store(true); }
  void record_surface_committed() { surface_committed_.store(true); }
  void record_buffer_scale(std::int32_t scale) { buffer_scale_.store(scale); }

  [[nodiscard]] bool wait_for_apply_count(std::uint32_t count) const {
    return wait_for([&] { return apply_count_.load() >= count; });
  }
  [[nodiscard]] bool wait_for_surface_ready() const {
    return wait_for([&] {
      return surface_set_.load() && buffer_attached_.load() &&
          surface_committed_.load();
    });
  }
  [[nodiscard]] WaylandCursorSurfaceState snapshot() const {
    return WaylandCursorSurfaceState{
        .apply_count = apply_count_.load(),
        .hotspot_x = hotspot_x_.load(),
        .hotspot_y = hotspot_y_.load(),
        .buffer_scale = buffer_scale_.load(),
        .surface_set = surface_set_.load(),
        .buffer_attached = buffer_attached_.load(),
        .surface_committed = surface_committed_.load(),
    };
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

  std::atomic_uint32_t apply_count_{0};
  std::atomic_int hotspot_x_{0};
  std::atomic_int hotspot_y_{0};
  std::atomic_int buffer_scale_{1};
  std::atomic_bool surface_set_{false};
  std::atomic_bool buffer_attached_{false};
  std::atomic_bool surface_committed_{false};
};

} // namespace cgpui::test
