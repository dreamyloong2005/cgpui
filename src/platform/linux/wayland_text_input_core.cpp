#include "wayland_text_input_internal.hpp"

namespace cgpui {

void WaylandTextInput::set_display(wl_display* display) {
  display_ = display;
}

void WaylandTextInput::set_manager(zwp_text_input_manager_v3* manager) {
  manager_ = manager;
}

void WaylandTextInput::set_window_lookup(WindowLookup lookup) {
  find_window_ = std::move(lookup);
}

void WaylandTextInput::set_modifiers_provider(ModifiersProvider provider) {
  modifiers_ = std::move(provider);
}

void WaylandTextInput::bind_to_seat(wl_seat* seat) {
  if (manager_ == nullptr || seat == nullptr || text_input_ != nullptr) {
    return;
  }

  text_input_ = zwp_text_input_manager_v3_get_text_input(manager_, seat);
  if (text_input_ == nullptr) {
    return;
  }

  static const zwp_text_input_v3_listener listener{
      .enter = &WaylandTextInput::handle_enter,
      .leave = &WaylandTextInput::handle_leave,
      .preedit_string = &WaylandTextInput::handle_preedit_string,
      .commit_string = &WaylandTextInput::handle_commit_string,
      .delete_surrounding_text =
          &WaylandTextInput::handle_delete_surrounding_text,
      .done = &WaylandTextInput::handle_done,
  };
  zwp_text_input_v3_add_listener(text_input_, &listener, this);
}

void WaylandTextInput::reset_text_input() {
  active_window_ = nullptr;
  pending_preedit_.reset();
  pending_commit_.reset();
  pending_delete_surrounding_.reset();
  if (text_input_ != nullptr) {
    zwp_text_input_v3_destroy(text_input_);
    text_input_ = nullptr;
  }
}

bool WaylandTextInput::available() const {
  return text_input_ != nullptr;
}

} // namespace cgpui
