#include "wayland_window_internal.hpp"

namespace cgpui {

PlatformWindowChromeState WaylandWindow::apply_window_chrome(
    WindowChromeOptions options) {
  const bool decoration_supported = decoration_ != nullptr;
  WindowChromeOptions applied = options;
  if (decoration_supported) {
    const bool server_side = options.decorations && options.titlebar_visible;
    zxdg_toplevel_decoration_v1_set_mode(
        decoration_, server_side
            ? zxdg_toplevel_decoration_mode_server_side
            : zxdg_toplevel_decoration_mode_client_side);
  } else {
    applied.titlebar_visible = true;
    applied.decorations = true;
  }
  const auto width = static_cast<std::int32_t>(logical_size_.width);
  const auto height = static_cast<std::int32_t>(logical_size_.height);
  xdg_toplevel_set_min_size(
      toplevel_, options.resizable ? 0 : width, options.resizable ? 0 : height);
  xdg_toplevel_set_max_size(
      toplevel_, options.resizable ? 0 : width, options.resizable ? 0 : height);
  chrome_state_ = WaylandWindowChromeState{
      .platform = PlatformWindowChromeState{
          .supported = decoration_supported,
          .decoration_control_supported = decoration_supported,
          .transparency_supported = true,
          .backend = "wayland",
          .requested = options,
          .applied = applied,
          .reason = decoration_supported
              ? std::string{}
              : "zxdg_decoration_manager_v1 global not available"},
      .xdg_decoration_supported = decoration_supported};
  wl_surface_commit(surface_);
  wl_display_flush(display_);
  return chrome_state_.platform;
}

void WaylandWindow::decoration_configured(std::uint32_t mode) {
  const bool server_side =
      mode == zxdg_toplevel_decoration_mode_server_side;
  chrome_state_.platform.applied.titlebar_visible = server_side;
  chrome_state_.platform.applied.decorations = server_side;
}

void WaylandWindow::handle_decoration_configure(
    void* data,
    zxdg_toplevel_decoration_v1*,
    std::uint32_t mode) {
  static_cast<WaylandWindow*>(data)->decoration_configured(mode);
}

} // namespace cgpui
