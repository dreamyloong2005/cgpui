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
}

bool WaylandWindow::acknowledge_configure(std::uint32_t serial) {
  const Size previous_framebuffer_size = state_.framebuffer_size;
  pending_configure_.pending_serial = serial;
  pending_configure_.last_acked_configure_serial = serial;
  pending_configure_.configured = true;
  pending_configure_.current_toplevel_state =
      pending_configure_.pending_toplevel_state;
  if (pending_configure_.pending_size) {
    logical_size_ = Size{
        static_cast<float>(pending_configure_.pending_width),
        static_cast<float>(pending_configure_.pending_height)};
    update_framebuffer_size();
  }
  pending_configure_.pending_size = false;
  return previous_framebuffer_size.width != state_.framebuffer_size.width ||
      previous_framebuffer_size.height != state_.framebuffer_size.height;
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
  const bool resized = window->acknowledge_configure(serial);
  (void)wl_display_flush(window->display_);
  window->configured_ = true;
  window->dispatch_configure_lifecycle_events(
      previous_state,
      window->pending_configure_.current_toplevel_state);
  if (was_configured && resized) {
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
  window->close_requested(WindowCloseRequestSource::window_manager);
}

} // namespace cgpui
