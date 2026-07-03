#include "wayland_text_input_internal.hpp"

namespace cgpui {

void WaylandTextInputDeleter::operator()(WaylandTextInput* text_input) const {
  delete text_input;
}

WaylandTextInputPtr create_wayland_text_input() {
  return WaylandTextInputPtr(new WaylandTextInput());
}

void wayland_text_input_set_display(
    WaylandTextInput& text_input,
    wl_display* display) {
  text_input.set_display(display);
}

void wayland_text_input_set_manager(
    WaylandTextInput& text_input,
    zwp_text_input_manager_v3* manager) {
  text_input.set_manager(manager);
}

void wayland_text_input_set_window_lookup(
    WaylandTextInput& text_input,
    std::function<WaylandWindow*(wl_surface*)> lookup) {
  text_input.set_window_lookup(std::move(lookup));
}

void wayland_text_input_set_modifiers_provider(
    WaylandTextInput& text_input,
    std::function<KeyboardModifiers()> provider) {
  text_input.set_modifiers_provider(std::move(provider));
}

void wayland_text_input_bind_to_seat(
    WaylandTextInput& text_input,
    wl_seat* seat) {
  text_input.bind_to_seat(seat);
}

void wayland_text_input_reset(WaylandTextInput& text_input) {
  text_input.reset_text_input();
}

bool wayland_text_input_available(const WaylandTextInput& text_input) {
  return text_input.available();
}

void wayland_text_input_apply_placement(
    WaylandTextInput& text_input,
    WaylandWindow& window) {
  text_input.apply_placement(window);
}

} // namespace cgpui
