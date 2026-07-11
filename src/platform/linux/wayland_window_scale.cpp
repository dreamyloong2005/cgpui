#include "wayland_window_internal.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {

void WaylandWindow::output_scale_changed(
    wl_output* output,
    std::int32_t,
    bool present) {
  if (!present) {
    std::erase(entered_outputs_, output);
    refresh_output_scale();
    return;
  }
  if (std::ranges::find(entered_outputs_, output) != entered_outputs_.end()) {
    refresh_output_scale();
  }
}

void WaylandWindow::refresh_output_scale() {
  float scale = 1.0F;
  if (preferred_fractional_scale_.has_value()) {
    scale = *preferred_fractional_scale_;
  } else {
    for (wl_output* output : entered_outputs_) {
      scale = std::max(
          scale, static_cast<float>(output_scale_lookup_(output)));
    }
  }
  if (state_.scale.value == scale) {
    return;
  }
  state_.scale = DpiScale{scale};
  wl_surface_set_buffer_scale(
      surface_, static_cast<std::int32_t>(std::ceil(scale)));
  wl_surface_commit(surface_);
  update_framebuffer_size();
  if (scale_changed_) scale_changed_(*this);
  if (configured_) {
    callback_(WindowResized{
        .size = state_.framebuffer_size,
        .scale = state_.scale});
  }
}

void WaylandWindow::update_framebuffer_size() {
  state_.framebuffer_size = Size{
      logical_size_.width * state_.scale.value,
      logical_size_.height * state_.scale.value};
  update_fractional_viewport_destination();
}

void WaylandWindow::handle_surface_enter(
    void* data,
    wl_surface*,
    wl_output* output) {
  auto* window = static_cast<WaylandWindow*>(data);
  if (std::ranges::find(window->entered_outputs_, output) ==
      window->entered_outputs_.end()) {
    window->entered_outputs_.push_back(output);
  }
  window->refresh_output_scale();
}

void WaylandWindow::handle_surface_leave(
    void* data,
    wl_surface*,
    wl_output* output) {
  auto* window = static_cast<WaylandWindow*>(data);
  std::erase(window->entered_outputs_, output);
  window->refresh_output_scale();
}

} // namespace cgpui
