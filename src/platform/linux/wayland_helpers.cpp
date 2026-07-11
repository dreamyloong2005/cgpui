#include "wayland_internal.hpp"

namespace cgpui {
namespace {

constexpr std::uint32_t linux_button_left = 0x110;
constexpr std::uint32_t linux_button_right = 0x111;
constexpr std::uint32_t linux_button_middle = 0x112;

} // namespace

Error wayland_error(ErrorCode code, std::string message) {
  return Error{.code = code, .message = std::move(message)};
}

MouseButton mouse_button_from_wayland(std::uint32_t button) {
  switch (button) {
    case linux_button_left:
      return MouseButton::left;
    case linux_button_right:
      return MouseButton::right;
    case linux_button_middle:
      return MouseButton::middle;
    default:
      return MouseButton::other;
  }
}

Point point_from_fixed(wl_fixed_t x, wl_fixed_t y) {
  return Point{
      static_cast<float>(wl_fixed_to_double(x)),
      static_cast<float>(wl_fixed_to_double(y)),
  };
}

KeyboardModifiers modifiers_from_xkb_state(xkb_state* state) {
  if (state == nullptr) {
    return {};
  }

  return KeyboardModifiers{
      .shift = xkb_state_mod_name_is_active(
                   state,
                   XKB_MOD_NAME_SHIFT,
                   XKB_STATE_MODS_EFFECTIVE) == 1,
      .control = xkb_state_mod_name_is_active(
                     state,
                     XKB_MOD_NAME_CTRL,
                     XKB_STATE_MODS_EFFECTIVE) == 1,
      .alt = xkb_state_mod_name_is_active(
                 state,
                 XKB_MOD_NAME_ALT,
                 XKB_STATE_MODS_EFFECTIVE) == 1,
      .super = xkb_state_mod_name_is_active(
                   state,
                   XKB_MOD_NAME_LOGO,
                   XKB_STATE_MODS_EFFECTIVE) == 1,
  };
}

} // namespace cgpui
