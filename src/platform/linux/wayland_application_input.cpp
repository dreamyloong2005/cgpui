#include "wayland_application_internal.hpp"

namespace cgpui {

void WaylandApplication::set_window_ime_text_input_placement(
    WaylandWindow& window,
    std::optional<ImeTextInputPlacement> placement) {
  wayland_window_set_ime_text_input_placement(window, std::move(placement));
  wayland_text_input_apply_placement(*text_input_, window);
}

} // namespace cgpui
