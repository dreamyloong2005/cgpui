#include "wayland_window_internal.hpp"

namespace cgpui {

Result<std::unique_ptr<WaylandWindow>> WaylandWindow::create(
    wl_display* display,
    wl_compositor* compositor,
    xdg_wm_base* shell,
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback,
    bool text_input_available,
    WaylandOutputScaleLookup output_scale_lookup) {
  auto window = std::unique_ptr<WaylandWindow>(
      new WaylandWindow(display, std::move(callback), WindowState{
          .framebuffer_size = descriptor.size,
          .scale = DpiScale{1.0F},
          .close_requested = false,
      }, std::move(output_scale_lookup)));
  window->set_text_input_available(text_input_available);

  auto initialized = window->initialize(compositor, shell, descriptor);
  if (!initialized) {
    return std::unexpected(initialized.error());
  }

  window->apply_window_chrome(descriptor.chrome);
  return window;
}

WaylandWindow::WaylandWindow(
    wl_display* display,
    PlatformEventCallback callback,
    WindowState state,
    WaylandOutputScaleLookup output_scale_lookup)
    : display_(display),
      callback_(std::move(callback)),
      state_(state),
      logical_size_(state.framebuffer_size),
      output_scale_lookup_(std::move(output_scale_lookup)) {}

WaylandWindow::~WaylandWindow() {
  if (toplevel_ != nullptr) {
    xdg_toplevel_destroy(toplevel_);
  }
  if (xdg_surface_ != nullptr) {
    xdg_surface_destroy(xdg_surface_);
  }
  if (surface_ != nullptr) {
    wl_surface_destroy(surface_);
  }
}

NativeSurfaceHandle WaylandWindow::native_surface() const {
  return WaylandSurfaceHandle{.display = display_, .surface = surface_};
}

wl_surface* WaylandWindow::surface() const {
  return surface_;
}

WindowState WaylandWindow::state() const {
  return state_;
}

void WaylandWindow::request_redraw() {
  wl_surface_commit(surface_);
  callback_(WindowRedrawRequested{});
}

void WaylandWindow::set_title(std::string_view title) {
  const std::string title_string(title);
  xdg_toplevel_set_title(toplevel_, title_string.c_str());
  wl_surface_commit(surface_);
}

PlatformWindowChromeState WaylandWindow::apply_window_chrome(
    WindowChromeOptions options) {
  const WindowChromeOptions applied{
      .titlebar_visible = true,
      .decorations = true,
      .resizable = true,
      .transparent_background = false,
  };
  chrome_state_ = WaylandWindowChromeState{
      .platform =
          PlatformWindowChromeState{
              .supported = false,
              .backend = "wayland",
              .requested = options,
              .applied = applied,
              .reason = "xdg-decoration window chrome is not implemented",
          },
      .xdg_decoration_supported = false,
  };
  return chrome_state_.platform;
}

void WaylandWindow::update_accessibility_tree(
    PlatformAccessibilityTreeUpdate update) {
  wayland_atspi_update_accessibility_tree(
      *atspi_accessibility_,
      std::move(update));
}

Result<void> WaylandWindow::initialize(
    wl_compositor* compositor,
    xdg_wm_base* shell,
    const WindowDescriptor& descriptor) {
  surface_ = wl_compositor_create_surface(compositor);
  if (surface_ == nullptr) {
    return std::unexpected(wayland_error(
        ErrorCode::window_creation_failed,
        "wl_compositor_create_surface failed"));
  }
  static const wl_surface_listener output_listener{
      .enter = &WaylandWindow::handle_surface_enter,
      .leave = &WaylandWindow::handle_surface_leave,
      .preferred_buffer_scale = nullptr,
      .preferred_buffer_transform = nullptr,
  };
  wl_surface_add_listener(surface_, &output_listener, this);

  xdg_surface_ = xdg_wm_base_get_xdg_surface(shell, surface_);
  if (xdg_surface_ == nullptr) {
    return std::unexpected(wayland_error(
        ErrorCode::window_creation_failed,
        "xdg_surface creation failed"));
  }
  static const xdg_surface_listener surface_listener{
      .configure = &WaylandWindow::handle_surface_configure,
  };
  xdg_surface_add_listener(xdg_surface_, &surface_listener, this);

  toplevel_ = xdg_surface_get_toplevel(xdg_surface_);
  if (toplevel_ == nullptr) {
    return std::unexpected(wayland_error(
        ErrorCode::window_creation_failed,
        "xdg_toplevel creation failed"));
  }
  static const xdg_toplevel_listener toplevel_listener{
      .configure = &WaylandWindow::handle_toplevel_configure,
      .close = &WaylandWindow::handle_toplevel_close,
  };
  xdg_toplevel_add_listener(toplevel_, &toplevel_listener, this);
  set_title(descriptor.title);

  wl_surface_commit(surface_);
  while (!configured_) {
    if (wl_display_dispatch(display_) == -1) {
      return std::unexpected(wayland_error(
          ErrorCode::window_creation_failed,
          "wl_display_dispatch failed while waiting for configure"));
    }
  }

  callback_(WindowResized{.size = state_.framebuffer_size, .scale = state_.scale});
  return {};
}

} // namespace cgpui
