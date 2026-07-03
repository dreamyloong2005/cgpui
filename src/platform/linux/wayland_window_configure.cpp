#include "wayland_window_internal.hpp"

namespace cgpui {

void WaylandWindow::record_toplevel_configure_state(
    std::int32_t width,
    std::int32_t height,
    wl_array* states) {
  pending_configure_.pending_width = width;
  pending_configure_.pending_height = height;
  pending_configure_.pending_size = width > 0 && height > 0;
  pending_configure_.pending_toplevel_state =
      parse_xdg_toplevel_states(states);
  if (pending_configure_.pending_size) {
    state_.framebuffer_size =
        Size{static_cast<float>(width), static_cast<float>(height)};
    if (configured_) {
      resize_pending_surface_configure_ = true;
    }
  }
}

void WaylandWindow::acknowledge_configure(std::uint32_t serial) {
  pending_configure_.pending_serial = serial;
  pending_configure_.last_acked_configure_serial = serial;
  pending_configure_.configured = true;
  pending_configure_.current_toplevel_state =
      pending_configure_.pending_toplevel_state;
}

void WaylandWindow::dispatch_configure_lifecycle_events(
    WaylandXdgToplevelState previous,
    WaylandXdgToplevelState current) {
  if (previous.activated != current.activated) {
    callback_(WindowActivated{.active = current.activated});
  }
  if ((previous.maximized || previous.fullscreen) && !current.maximized &&
      !current.fullscreen) {
    callback_(WindowRestored{});
  }
}

void WaylandWindow::handle_surface_configure(
    void* data,
    xdg_surface* surface,
    std::uint32_t serial) {
  auto* window = static_cast<WaylandWindow*>(data);
  const bool was_configured = window->configured_;
  const WaylandXdgToplevelState previous_state =
      window->pending_configure_.current_toplevel_state;
  xdg_surface_ack_configure(surface, serial);
  (void)wl_display_flush(window->display_);
  window->acknowledge_configure(serial);
  window->configured_ = true;
  window->dispatch_configure_lifecycle_events(
      previous_state,
      window->pending_configure_.current_toplevel_state);
  if (was_configured && window->resize_pending_surface_configure_) {
    window->resize_pending_surface_configure_ = false;
    window->callback_(WindowResized{
        .size = window->state_.framebuffer_size,
        .scale = window->state_.scale});
  }
}

void WaylandWindow::handle_toplevel_configure(
    void* data,
    xdg_toplevel* toplevel,
    std::int32_t width,
    std::int32_t height,
    wl_array* states) {
  (void)toplevel;
  auto* window = static_cast<WaylandWindow*>(data);
  window->record_toplevel_configure_state(width, height, states);
}

void WaylandWindow::handle_toplevel_close(
    void* data,
    xdg_toplevel* toplevel) {
  (void)toplevel;
  auto* window = static_cast<WaylandWindow*>(data);
  window->close_requested();
}

} // namespace cgpui
