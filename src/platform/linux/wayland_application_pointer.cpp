#include "wayland_application_internal.hpp"

namespace cgpui {

void WaylandApplication::handle_pointer_enter(
    void* data,
    wl_pointer* pointer,
    std::uint32_t serial,
    wl_surface* surface,
    wl_fixed_t surface_x,
    wl_fixed_t surface_y) {
  (void)pointer;
  auto* app = static_cast<WaylandApplication*>(data);
  app->pointer_enter_serial_ = serial;
  app->pointer_window_ = app->find_window(surface);
  app->pointer_position_ = point_from_fixed(surface_x, surface_y);
  app->apply_cursor_for(app->pointer_window_);
  if (app->pointer_window_ != nullptr) {
    wayland_window_pointer_moved(*app->pointer_window_, app->pointer_position_);
  }
}

void WaylandApplication::handle_pointer_leave(
    void* data,
    wl_pointer* pointer,
    std::uint32_t serial,
    wl_surface* surface) {
  (void)pointer;
  (void)serial;
  auto* app = static_cast<WaylandApplication*>(data);
  if (app->pointer_window_ != nullptr &&
      wayland_window_surface(*app->pointer_window_) == surface) {
    WaylandWindow* window = app->pointer_window_;
    app->pointer_window_ = nullptr;
    app->pointer_enter_serial_ = 0;
    app->pointer_scroll_frame_ = {};
    wayland_window_pointer_exited(*window, app->pointer_position_);
  }
}

void WaylandApplication::handle_pointer_motion(
    void* data,
    wl_pointer* pointer,
    std::uint32_t time,
    wl_fixed_t surface_x,
    wl_fixed_t surface_y) {
  (void)pointer;
  (void)time;
  auto* app = static_cast<WaylandApplication*>(data);
  app->pointer_position_ = point_from_fixed(surface_x, surface_y);
  if (app->pointer_window_ != nullptr) {
    wayland_window_pointer_moved(*app->pointer_window_, app->pointer_position_);
  }
}

void WaylandApplication::handle_pointer_button(
    void* data,
    wl_pointer* pointer,
    std::uint32_t serial,
    std::uint32_t time,
    std::uint32_t button,
    std::uint32_t state) {
  (void)pointer;
  (void)serial;
  (void)time;
  auto* app = static_cast<WaylandApplication*>(data);
  if (app->pointer_window_ != nullptr) {
    wayland_window_pointer_button(
        *app->pointer_window_,
        mouse_button_from_wayland(button),
        state == WL_POINTER_BUTTON_STATE_PRESSED,
        app->pointer_position_);
  }
}

} // namespace cgpui
