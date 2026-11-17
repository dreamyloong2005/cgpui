#include "runtime_window_title_bar_internal.hpp"

#include "cgpui/ui/paint.hpp"

#include <utility>

namespace cgpui {

void runtime_publish_window_control_regions(
    PlatformWindow* window,
    const ElementTree* tree,
    DpiScale scale,
    std::vector<WindowControlRegion> regions) {
  if (window == nullptr) return;
  if (tree != nullptr && window->frame_metrics().client_title_bar) {
    PaintList scratch;
    scratch.set_scale(scale);
    tree->paint(scratch);
    const auto painted = scratch.window_control_regions();
    regions.insert(regions.end(), painted.begin(), painted.end());
  }
  window->set_window_control_regions(std::move(regions));
}

} // namespace cgpui
