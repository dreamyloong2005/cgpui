#include "wayland_application_internal.hpp"

namespace cgpui {

Result<std::unique_ptr<PlatformWindow>> WaylandApplication::create_window(
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback) {
  return create_window_with_parent(descriptor, std::move(callback), nullptr);
}

Result<std::unique_ptr<PlatformWindow>>
WaylandApplication::create_window_with_parent(
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback,
    xdg_toplevel* parent) {
  if (display_ == nullptr) {
    return std::unexpected(wayland_error(
        ErrorCode::platform_initialization_failed,
        "wl_display_connect failed"));
  }
  if (!initialization_error_.empty()) {
    return std::unexpected(wayland_error(
        ErrorCode::platform_initialization_failed,
        initialization_error_));
  }
  if (compositor_ == nullptr) {
    return std::unexpected(wayland_error(
        ErrorCode::platform_initialization_failed,
        "wl_compositor global not available"));
  }
  if (shell_ == nullptr) {
    return std::unexpected(wayland_error(
        ErrorCode::platform_initialization_failed,
        "xdg_wm_base global not available"));
  }

  auto window = create_wayland_window(
      display_,
      compositor_,
      shell_,
      decoration_manager_,
      WaylandFractionalScaleGlobals{
          .manager = fractional_scale_manager_, .viewporter = viewporter_},
      parent,
      descriptor,
      std::move(callback),
      wayland_text_input_available(*text_input_),
      [this](wl_output* output) {
        return output_scales_.scale_for(output);
      },
      [this](WaylandWindow& scaled_window) {
        cursor_scale_changed(scaled_window);
      });
  if (!window) {
    return std::unexpected(window.error());
  }

  WaylandWindow* raw_window = window->get();
  register_window(raw_window);
  return make_registered_wayland_window(
      std::move(*window),
      [this](WaylandWindow* registered_window) {
        unregister_window(registered_window);
      },
      [this](WaylandWindow& registered_window, CursorShape cursor_shape) {
        set_window_cursor(registered_window, cursor_shape);
      },
      [this](
          WaylandWindow& registered_window,
          std::optional<ImeTextInputPlacement> placement) {
        set_window_ime_text_input_placement(
            registered_window,
            std::move(placement));
      });
}

} // namespace cgpui
