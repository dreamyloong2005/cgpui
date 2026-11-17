#pragma once

#include <atomic>
#include <cstdint>

struct wl_display;

namespace cgpui::test {

// Server side of a fake ext_background_effect_manager_v1 global for the test
// compositor. Counters are written on the server thread.
struct WaylandFakeBackgroundEffect {
  std::uint32_t capabilities = 1;
  std::atomic_int effects_created{0};
  std::atomic_int blur_regions_set{0};
  std::atomic_int blur_regions_cleared{0};

  void install(wl_display* display);
};

} // namespace cgpui::test
