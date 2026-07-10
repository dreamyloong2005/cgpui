#include "wayland_window_internal.hpp"

namespace cgpui {
namespace {

PlatformWindowDisplayState display_state_for(
    WaylandXdgToplevelState state) {
  if (state.fullscreen) {
    return PlatformWindowDisplayState::fullscreen;
  }
  if (state.maximized) {
    return PlatformWindowDisplayState::maximized;
  }
  return PlatformWindowDisplayState::normal;
}

} // namespace

PlatformWindowLifecycleState WaylandWindow::lifecycle_state() const {
  const WaylandXdgToplevelState current =
      pending_configure_.current_toplevel_state;
  return PlatformWindowLifecycleState{
      .native_window_created =
          surface_ != nullptr && xdg_surface_ != nullptr && toplevel_ != nullptr,
      .initial_configure_complete =
          configured_ && pending_configure_.configured &&
          pending_configure_.last_acked_configure_serial != 0,
      .active = current.activated,
      .focused = focused_,
      .close_requested = state_.close_requested,
      .display_state = display_state_for(current),
  };
}

} // namespace cgpui
