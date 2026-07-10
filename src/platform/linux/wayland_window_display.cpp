#include "wayland_window_internal.hpp"

namespace cgpui {

bool WaylandWindow::request_display_state(
    PlatformWindowDisplayState display_state) {
  if (surface_ == nullptr || toplevel_ == nullptr) {
    return false;
  }
  const WaylandXdgToplevelState current =
      pending_configure_.current_toplevel_state;
  switch (display_state) {
  case PlatformWindowDisplayState::normal:
    if (current.fullscreen) {
      xdg_toplevel_unset_fullscreen(toplevel_);
    }
    if (current.maximized) {
      xdg_toplevel_unset_maximized(toplevel_);
    }
    break;
  case PlatformWindowDisplayState::minimized:
    xdg_toplevel_set_minimized(toplevel_);
    break;
  case PlatformWindowDisplayState::maximized:
    if (current.fullscreen) {
      xdg_toplevel_unset_fullscreen(toplevel_);
    }
    if (!current.maximized) {
      xdg_toplevel_set_maximized(toplevel_);
    }
    break;
  case PlatformWindowDisplayState::fullscreen:
    if (current.maximized) {
      xdg_toplevel_unset_maximized(toplevel_);
    }
    if (!current.fullscreen) {
      xdg_toplevel_set_fullscreen(toplevel_, nullptr);
    }
    break;
  }
  wl_surface_commit(surface_);
  (void)wl_display_flush(display_);
  return true;
}

} // namespace cgpui
