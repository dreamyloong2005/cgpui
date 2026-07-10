#include "wayland_window_internal.hpp"

namespace cgpui {

RegisteredWaylandWindow::RegisteredWaylandWindow(
    WaylandWindowPtr window,
    WaylandWindowUnregisterCallback unregister,
    WaylandWindowCursorCallback set_cursor,
    WaylandWindowImePlacementCallback set_ime_placement)
    : window_(std::move(window)),
      unregister_(std::move(unregister)),
      set_cursor_(std::move(set_cursor)),
      set_ime_placement_(std::move(set_ime_placement)) {}

RegisteredWaylandWindow::~RegisteredWaylandWindow() {
  if (unregister_) {
    unregister_(window_.get());
  }
}

NativeSurfaceHandle RegisteredWaylandWindow::native_surface() const {
  return window_->native_surface();
}

WindowState RegisteredWaylandWindow::state() const {
  return window_->state();
}

PlatformWindowLifecycleState RegisteredWaylandWindow::lifecycle_state() const {
  return window_->lifecycle_state();
}

bool RegisteredWaylandWindow::request_display_state(
    PlatformWindowDisplayState display_state) {
  return window_->request_display_state(display_state);
}

PlatformWindowPositionState RegisteredWaylandWindow::position_state() const {
  return window_->position_state();
}

bool RegisteredWaylandWindow::request_position(Point position) {
  return window_->request_position(position);
}

PlatformWindowCloseState RegisteredWaylandWindow::close_request_state() const {
  return window_->close_request_state();
}

bool RegisteredWaylandWindow::resolve_close_request(
    PlatformWindowCloseResolution resolution) {
  return window_->resolve_close_request(resolution);
}

void RegisteredWaylandWindow::request_redraw() {
  window_->request_redraw();
}

void RegisteredWaylandWindow::request_close() {
  window_->request_close();
}

void RegisteredWaylandWindow::set_title(std::string_view title) {
  window_->set_title(title);
}

void RegisteredWaylandWindow::set_cursor(CursorShape cursor_shape) {
  if (set_cursor_) {
    set_cursor_(*window_, cursor_shape);
  }
}

void RegisteredWaylandWindow::set_ime_text_input_placement(
    std::optional<ImeTextInputPlacement> placement) {
  if (set_ime_placement_) {
    set_ime_placement_(*window_, std::move(placement));
  }
}

PlatformWindowChromeState RegisteredWaylandWindow::apply_window_chrome(
    WindowChromeOptions options) {
  return window_->apply_window_chrome(options);
}

void RegisteredWaylandWindow::update_accessibility_tree(
    PlatformAccessibilityTreeUpdate update) {
  window_->update_accessibility_tree(std::move(update));
}

void WaylandWindowDeleter::operator()(WaylandWindow* window) const {
  delete window;
}

Result<WaylandWindowPtr> create_wayland_window(
    wl_display* display,
    wl_compositor* compositor,
    xdg_wm_base* shell,
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback,
    bool text_input_available,
    WaylandOutputScaleLookup output_scale_lookup) {
  auto window = WaylandWindow::create(
      display,
      compositor,
      shell,
      descriptor,
      std::move(callback),
      text_input_available,
      std::move(output_scale_lookup));
  if (!window) {
    return std::unexpected(window.error());
  }
  return WaylandWindowPtr(window->release());
}

std::unique_ptr<PlatformWindow> make_registered_wayland_window(
    WaylandWindowPtr window,
    WaylandWindowUnregisterCallback unregister,
    WaylandWindowCursorCallback set_cursor,
    WaylandWindowImePlacementCallback set_ime_placement) {
  return std::make_unique<RegisteredWaylandWindow>(
      std::move(window),
      std::move(unregister),
      std::move(set_cursor),
      std::move(set_ime_placement));
}

} // namespace cgpui
