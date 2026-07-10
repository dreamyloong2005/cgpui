#include "wayland_application_internal.hpp"

namespace cgpui {

WaylandApplication::WaylandApplication()
    : display_(wl_display_connect(nullptr)),
      data_device_(create_wayland_data_device()),
      text_input_(create_wayland_text_input()),
      native_menu_state_(create_wayland_native_menu_state()),
      native_file_dialog_state_(create_wayland_native_file_dialog_state()) {
  if (display_ == nullptr) {
    return;
  }
  wayland_data_device_set_display(*data_device_, display_);
  wayland_text_input_set_display(*text_input_, display_);
  if (pipe2(wakeup_pipe_, O_NONBLOCK | O_CLOEXEC) == -1) {
    initialization_error_ = "pipe2 failed while creating event-loop wakeup pipe";
    return;
  }
  configure_data_device_lookup();
  configure_text_input_lookup();
  configure_output_scale_registry();

  registry_ = wl_display_get_registry(display_);
  if (registry_ == nullptr) {
    initialization_error_ = "wl_display_get_registry failed";
    return;
  }

  static const wl_registry_listener registry_listener{
      .global = &WaylandApplication::handle_global,
      .global_remove = &WaylandApplication::handle_global_remove,
  };
  wl_registry_add_listener(registry_, &registry_listener, this);
  if (wl_display_roundtrip(display_) == -1) {
    initialization_error_ = "wl_display_roundtrip failed";
    return;
  }

  if (shell_ != nullptr) {
    static const xdg_wm_base_listener shell_listener{
        .ping = &WaylandApplication::handle_shell_ping,
    };
    xdg_wm_base_add_listener(shell_, &shell_listener, this);
  }
  if (seat_ != nullptr) {
    static const wl_seat_listener seat_listener{
        .capabilities = &WaylandApplication::handle_seat_capabilities,
        .name = &WaylandApplication::handle_seat_name,
    };
    wl_seat_add_listener(seat_, &seat_listener, this);
    wayland_data_device_bind_to_seat(*data_device_, seat_);
    wayland_text_input_bind_to_seat(*text_input_, seat_);
    if (wl_display_roundtrip(display_) == -1) {
      initialization_error_ = "wl_display_roundtrip failed while waiting for seat";
    }
  }
}

WaylandApplication::~WaylandApplication() {
  wayland_text_input_reset(*text_input_);
  wayland_data_device_reset(*data_device_);
  wayland_keyboard_reset(keyboard_state_);
  if (keyboard_ != nullptr) {
    wl_keyboard_destroy(keyboard_);
  }
  if (pointer_ != nullptr) {
    wl_pointer_destroy(pointer_);
  }
  if (seat_ != nullptr) {
    wl_seat_destroy(seat_);
  }
  output_scales_.reset();
  if (data_device_manager_ != nullptr) {
    wl_data_device_manager_destroy(data_device_manager_);
  }
  if (text_input_manager_ != nullptr) {
    zwp_text_input_manager_v3_destroy(text_input_manager_);
  }
  if (shell_ != nullptr) {
    xdg_wm_base_destroy(shell_);
  }
  if (compositor_ != nullptr) {
    wl_compositor_destroy(compositor_);
  }
  if (registry_ != nullptr) {
    wl_registry_destroy(registry_);
  }
  if (display_ != nullptr) {
    wl_display_disconnect(display_);
  }
  if (wakeup_pipe_[0] != -1) {
    close(wakeup_pipe_[0]);
  }
  if (wakeup_pipe_[1] != -1) {
    close(wakeup_pipe_[1]);
  }
}

} // namespace cgpui
