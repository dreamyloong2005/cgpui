#include "x11_keyboard_internal.hpp"
#include "x11_window_internal.hpp"

namespace cgpui {

void X11Window::handle_keyboard_event(const xcb_generic_event_t& event) {
  if (keyboard_ == nullptr) return;
  const bool pressed = (event.response_type & 0x7f) == XCB_KEY_PRESS;
  const auto& key = reinterpret_cast<const xcb_key_press_event_t&>(event);
  X11KeyTranslation translated =
      x11_translate_key(*keyboard_, key.detail, pressed);
  callback_(translated.key);
  if (!translated.text.empty()) {
    callback_(TextInput{
        .text = std::move(translated.text),
        .modifiers = translated.key.modifiers,
        .composed = false,
    });
  }
}

}  // namespace cgpui
