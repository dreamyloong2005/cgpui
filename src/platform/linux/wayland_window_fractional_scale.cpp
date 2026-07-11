#include "wayland_window_internal.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {

void WaylandWindow::initialize_fractional_scale(
    wp_fractional_scale_manager_v1* manager,
    wp_viewporter* viewporter) {
  if (manager == nullptr || viewporter == nullptr) {
    return;
  }
  fractional_scale_ =
      wp_fractional_scale_manager_v1_get_fractional_scale(manager, surface_);
  viewport_ = wp_viewporter_get_viewport(viewporter, surface_);
  if (fractional_scale_ == nullptr || viewport_ == nullptr) {
    return;
  }
  static const wp_fractional_scale_v1_listener listener{
      .preferred_scale = &WaylandWindow::handle_preferred_fractional_scale,
  };
  wp_fractional_scale_v1_add_listener(fractional_scale_, &listener, this);
  update_fractional_viewport_destination();
}

void WaylandWindow::update_fractional_viewport_destination() {
  if (viewport_ == nullptr) {
    return;
  }
  wp_viewport_set_destination(
      viewport_,
      static_cast<std::int32_t>(std::lround(logical_size_.width)),
      static_cast<std::int32_t>(std::lround(logical_size_.height)));
}

void WaylandWindow::handle_preferred_fractional_scale(
    void* data,
    wp_fractional_scale_v1*,
    std::uint32_t scale) {
  auto* window = static_cast<WaylandWindow*>(data);
  window->preferred_fractional_scale_ =
      std::max(static_cast<float>(scale) / 120.0F, 1.0F);
  window->refresh_output_scale();
}

} // namespace cgpui
