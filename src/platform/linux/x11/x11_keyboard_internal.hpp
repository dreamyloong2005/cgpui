#pragma once

#include "x11_internal.hpp"

#include <xkbcommon/xkbcommon.h>

namespace cgpui {

struct X11KeyboardState {
  ~X11KeyboardState();
  xkb_context* context = nullptr;
  xkb_keymap* keymap = nullptr;
  xkb_state* state = nullptr;
  std::int32_t device_id = -1;
};

struct X11KeyTranslation {
  KeyboardKey key;
  std::string text;
};

[[nodiscard]] Result<std::shared_ptr<X11KeyboardState>>
create_x11_keyboard_state(xcb_connection_t* connection);
[[nodiscard]] X11KeyTranslation x11_translate_key(
    X11KeyboardState& keyboard,
    std::uint8_t keycode,
    bool pressed);

}  // namespace cgpui
