#include "wayland_application_internal.hpp"

namespace cgpui {

void WaylandApplication::handle_pointer_axis(
    void* data,
    wl_pointer* pointer,
    std::uint32_t time,
    std::uint32_t axis,
    wl_fixed_t value) {
  auto* app = static_cast<WaylandApplication*>(data);
  (void)time;
  const auto delta = static_cast<float>(wl_fixed_to_double(value));
  if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
    app->pending_scroll_delta_.y += delta;
  } else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
    app->pending_scroll_delta_.x += delta;
  } else {
    return;
  }

  app->pointer_scroll_pending_ = true;
  const auto pointer_version =
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(pointer));
  if (pointer_version < WL_POINTER_FRAME_SINCE_VERSION) {
    app->dispatch_pointer_scroll();
  }
}

void WaylandApplication::handle_pointer_frame(
    void* data,
    wl_pointer* pointer) {
  (void)pointer;
  auto* app = static_cast<WaylandApplication*>(data);
  app->dispatch_pointer_scroll();
}

void WaylandApplication::handle_pointer_axis_source(
    void* data,
    wl_pointer* pointer,
    std::uint32_t axis_source) {
  (void)data;
  (void)pointer;
  (void)axis_source;
}

void WaylandApplication::handle_pointer_axis_stop(
    void* data,
    wl_pointer* pointer,
    std::uint32_t time,
    std::uint32_t axis) {
  (void)data;
  (void)pointer;
  (void)time;
  (void)axis;
}

void WaylandApplication::handle_pointer_axis_discrete(
    void* data,
    wl_pointer* pointer,
    std::uint32_t axis,
    std::int32_t discrete) {
  (void)data;
  (void)pointer;
  (void)axis;
  (void)discrete;
}

void WaylandApplication::handle_pointer_axis_value120(
    void* data,
    wl_pointer* pointer,
    std::uint32_t axis,
    std::int32_t value120) {
  (void)data;
  (void)pointer;
  (void)axis;
  (void)value120;
}

void WaylandApplication::handle_pointer_axis_relative_direction(
    void* data,
    wl_pointer* pointer,
    std::uint32_t axis,
    std::uint32_t direction) {
  (void)data;
  (void)pointer;
  (void)axis;
  (void)direction;
}

void WaylandApplication::dispatch_pointer_scroll() {
  if (!pointer_scroll_pending_) {
    return;
  }

  const Point delta = pending_scroll_delta_;
  pending_scroll_delta_ = {};
  pointer_scroll_pending_ = false;
  if (pointer_window_ != nullptr) {
    wayland_window_pointer_scrolled(*pointer_window_, delta, pointer_position_);
  }
}

} // namespace cgpui
