#include "x11_keyboard_internal.hpp"

#include <xkbcommon/xkbcommon-x11.h>

#include <array>

namespace cgpui {
namespace {

KeyboardModifiers modifiers_for(xkb_state* state) {
  const auto active = [state](const char* name) {
    return xkb_state_mod_name_is_active(
        state, name, XKB_STATE_MODS_EFFECTIVE) > 0;
  };
  return KeyboardModifiers{
      .shift = active(XKB_MOD_NAME_SHIFT),
      .control = active(XKB_MOD_NAME_CTRL),
      .alt = active(XKB_MOD_NAME_ALT),
      .super = active(XKB_MOD_NAME_LOGO),
  };
}

}  // namespace

X11KeyboardState::~X11KeyboardState() {
  if (state != nullptr) xkb_state_unref(state);
  if (keymap != nullptr) xkb_keymap_unref(keymap);
  if (context != nullptr) xkb_context_unref(context);
}

Result<std::shared_ptr<X11KeyboardState>> create_x11_keyboard_state(
    xcb_connection_t* connection) {
  std::uint16_t major = 0;
  std::uint16_t minor = 0;
  std::uint8_t event = 0;
  std::uint8_t error = 0;
  if (!xkb_x11_setup_xkb_extension(
          connection,
          XKB_X11_MIN_MAJOR_XKB_VERSION,
          XKB_X11_MIN_MINOR_XKB_VERSION,
          XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
          &major, &minor, &event, &error)) {
    return std::unexpected(x11_error(
        ErrorCode::platform_initialization_failed,
        "XKB extension setup failed for X11"));
  }
  auto keyboard = std::make_shared<X11KeyboardState>();
  keyboard->context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  keyboard->device_id = xkb_x11_get_core_keyboard_device_id(connection);
  if (keyboard->context != nullptr && keyboard->device_id >= 0) {
    keyboard->keymap = xkb_x11_keymap_new_from_device(
        keyboard->context, connection, keyboard->device_id,
        XKB_KEYMAP_COMPILE_NO_FLAGS);
  }
  if (keyboard->keymap != nullptr) {
    keyboard->state = xkb_x11_state_new_from_device(
        keyboard->keymap, connection, keyboard->device_id);
  }
  if (keyboard->context == nullptr || keyboard->keymap == nullptr ||
      keyboard->state == nullptr) {
    return std::unexpected(x11_error(
        ErrorCode::platform_initialization_failed,
        "XKB keymap initialization failed for X11"));
  }
  return keyboard;
}

X11KeyTranslation x11_translate_key(
    X11KeyboardState& keyboard,
    std::uint8_t keycode,
    bool pressed) {
  xkb_state_update_key(
      keyboard.state, keycode,
      pressed ? XKB_KEY_DOWN : XKB_KEY_UP);
  X11KeyTranslation translated{
      .key = KeyboardKey{
          .key_code = keycode >= 8 ? static_cast<std::uint32_t>(keycode - 8) : keycode,
          .action = pressed ? KeyAction::pressed : KeyAction::released,
          .modifiers = modifiers_for(keyboard.state),
          .scan_code = keycode,
      },
  };
  if (pressed) {
    std::array<char, 64> buffer{};
    const int length = xkb_state_key_get_utf8(
        keyboard.state, keycode, buffer.data(), buffer.size());
    if (length > 0 && static_cast<std::size_t>(length) < buffer.size()) {
      translated.text.assign(buffer.data(), static_cast<std::size_t>(length));
    }
  }
  return translated;
}

}  // namespace cgpui
