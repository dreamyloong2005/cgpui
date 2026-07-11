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
  wayland_pointer_scroll_axis(app->pointer_scroll_frame_, axis, delta);
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
  (void)pointer;
  auto* app = static_cast<WaylandApplication*>(data);
  wayland_pointer_scroll_source(app->pointer_scroll_frame_, axis_source);
}

void WaylandApplication::handle_pointer_axis_stop(
    void* data,
    wl_pointer* pointer,
    std::uint32_t time,
    std::uint32_t axis) {
  (void)pointer;
  (void)time;
  auto* app = static_cast<WaylandApplication*>(data);
  wayland_pointer_scroll_stop(app->pointer_scroll_frame_, axis);
}

void WaylandApplication::handle_pointer_axis_discrete(
    void* data,
    wl_pointer* pointer,
    std::uint32_t axis,
    std::int32_t discrete) {
  (void)pointer;
  auto* app = static_cast<WaylandApplication*>(data);
  wayland_pointer_scroll_discrete(app->pointer_scroll_frame_, axis, discrete);
}

void WaylandApplication::handle_pointer_axis_value120(
    void* data,
    wl_pointer* pointer,
    std::uint32_t axis,
    std::int32_t value120) {
  (void)pointer;
  auto* app = static_cast<WaylandApplication*>(data);
  wayland_pointer_scroll_value120(app->pointer_scroll_frame_, axis, value120);
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
  const WaylandPointerScrollFrameResult frame =
      wayland_pointer_scroll_consume(pointer_scroll_frame_);
  if (!frame.publish) {
    return;
  }
  if (pointer_window_ != nullptr) {
    wayland_window_pointer_scrolled(
        *pointer_window_, frame.delta, pointer_position_, frame.precise);
  }
}

} // namespace cgpui
