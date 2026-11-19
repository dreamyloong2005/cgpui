#include "runtime_window_title_bar_internal.hpp"

#include <utility>

namespace cgpui {

void runtime_publish_window_control_regions(
    PlatformWindow* window,
    std::vector<WindowControlRegion> regions) {
  if (window == nullptr) return;
  window->set_window_control_regions(std::move(regions));
}

} // namespace cgpui
