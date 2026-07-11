#include "wayland_application_internal.hpp"

namespace cgpui {
namespace {

void release_pointer(wl_pointer* pointer) {
  const auto version =
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(pointer));
  if (version >= WL_POINTER_RELEASE_SINCE_VERSION) {
    wl_pointer_release(pointer);
  } else {
    wl_pointer_destroy(pointer);
  }
}

void release_keyboard(wl_keyboard* keyboard) {
  const auto version =
      wl_proxy_get_version(reinterpret_cast<wl_proxy*>(keyboard));
  if (version >= WL_KEYBOARD_RELEASE_SINCE_VERSION) {
    wl_keyboard_release(keyboard);
  } else {
    wl_keyboard_destroy(keyboard);
  }
}

} // namespace

void WaylandApplication::handle_seat_capabilities(
    void* data,
    wl_seat* seat,
    std::uint32_t capabilities) {
  auto* app = static_cast<WaylandApplication*>(data);
  const bool has_pointer = (capabilities & WL_SEAT_CAPABILITY_POINTER) != 0U;
  const bool has_keyboard = (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) != 0U;

  if (has_pointer) {
    if (app->pointer_ == nullptr) {
      app->pointer_ = wl_seat_get_pointer(seat);
      static const wl_pointer_listener pointer_listener{
          .enter = &WaylandApplication::handle_pointer_enter,
          .leave = &WaylandApplication::handle_pointer_leave,
          .motion = &WaylandApplication::handle_pointer_motion,
          .button = &WaylandApplication::handle_pointer_button,
          .axis = &WaylandApplication::handle_pointer_axis,
          .frame = &WaylandApplication::handle_pointer_frame,
          .axis_source = &WaylandApplication::handle_pointer_axis_source,
          .axis_stop = &WaylandApplication::handle_pointer_axis_stop,
          .axis_discrete = &WaylandApplication::handle_pointer_axis_discrete,
          .axis_value120 = &WaylandApplication::handle_pointer_axis_value120,
          .axis_relative_direction =
              &WaylandApplication::handle_pointer_axis_relative_direction,
      };
      wl_pointer_add_listener(app->pointer_, &pointer_listener, app);
    }
  } else if (app->pointer_ != nullptr) {
    release_pointer(app->pointer_);
    app->pointer_ = nullptr;
    app->pointer_window_ = nullptr;
    app->pointer_position_ = {};
    app->pointer_enter_serial_ = 0;
    app->pointer_scroll_frame_ = {};
  }

  if (capabilities != 0U) {
    wayland_data_device_bind_to_seat(*app->data_device_, seat);
  } else {
    wayland_data_device_reset(*app->data_device_);
  }
  if (has_keyboard) {
    wayland_text_input_bind_to_seat(*app->text_input_, seat);
  } else {
    wayland_text_input_reset(*app->text_input_);
  }

  if (has_keyboard) {
    if (app->keyboard_ == nullptr) {
      app->keyboard_ = wl_seat_get_keyboard(seat);
      static const wl_keyboard_listener keyboard_listener{
          .keymap = &WaylandApplication::handle_keyboard_keymap,
          .enter = &WaylandApplication::handle_keyboard_enter,
          .leave = &WaylandApplication::handle_keyboard_leave,
          .key = &WaylandApplication::handle_keyboard_key,
          .modifiers = &WaylandApplication::handle_keyboard_modifiers,
          .repeat_info = &WaylandApplication::handle_keyboard_repeat_info,
      };
      wl_keyboard_add_listener(app->keyboard_, &keyboard_listener, app);
    }
  } else if (app->keyboard_ != nullptr) {
    if (app->keyboard_window_ != nullptr) {
      wayland_window_focus_changed(*app->keyboard_window_, false);
      app->keyboard_window_ = nullptr;
    }
    release_keyboard(app->keyboard_);
    app->keyboard_ = nullptr;
    wayland_keyboard_reset(app->keyboard_state_);
  }
}

void WaylandApplication::handle_seat_name(
    void* data,
    wl_seat* seat,
    const char* name) {
  (void)data;
  (void)seat;
  (void)name;
}

} // namespace cgpui
