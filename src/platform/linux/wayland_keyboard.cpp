#include "wayland_internal.hpp"

namespace cgpui {
namespace {

void release_xkb_resources(WaylandKeyboardState& keyboard) {
  if (keyboard.state != nullptr) {
    xkb_state_unref(keyboard.state);
    keyboard.state = nullptr;
  }
  if (keyboard.keymap != nullptr) {
    xkb_keymap_unref(keyboard.keymap);
    keyboard.keymap = nullptr;
  }
  if (keyboard.context != nullptr) {
    xkb_context_unref(keyboard.context);
    keyboard.context = nullptr;
  }
}

void apply_modifier_state(WaylandKeyboardState& keyboard) {
  if (keyboard.state == nullptr) {
    return;
  }
  xkb_state_update_mask(
      keyboard.state,
      keyboard.mods_depressed,
      keyboard.mods_latched,
      keyboard.mods_locked,
      0,
      0,
      keyboard.layout_group);
}

} // namespace

void wayland_keyboard_load_keymap(
    WaylandKeyboardState& keyboard,
    std::uint32_t format,
    std::int32_t fd,
    std::uint32_t size) {
  if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 || size == 0) {
    close(fd);
    return;
  }

  void* mapped = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (mapped == MAP_FAILED) {
    close(fd);
    return;
  }

  auto* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  xkb_keymap* keymap = nullptr;
  xkb_state* state = nullptr;
  if (context != nullptr) {
    keymap = xkb_keymap_new_from_string(
        context,
        static_cast<const char*>(mapped),
        XKB_KEYMAP_FORMAT_TEXT_V1,
        XKB_KEYMAP_COMPILE_NO_FLAGS);
  }
  if (keymap != nullptr) {
    state = xkb_state_new(keymap);
  }

  munmap(mapped, size);
  close(fd);

  if (context == nullptr || keymap == nullptr || state == nullptr) {
    if (state != nullptr) {
      xkb_state_unref(state);
    }
    if (keymap != nullptr) {
      xkb_keymap_unref(keymap);
    }
    if (context != nullptr) {
      xkb_context_unref(context);
    }
    return;
  }

  release_xkb_resources(keyboard);
  keyboard.context = context;
  keyboard.keymap = keymap;
  keyboard.state = state;
  apply_modifier_state(keyboard);
}

void wayland_keyboard_reset(WaylandKeyboardState& keyboard) {
  release_xkb_resources(keyboard);
  keyboard.mods_depressed = 0;
  keyboard.mods_latched = 0;
  keyboard.mods_locked = 0;
  keyboard.layout_group = 0;
}

void wayland_keyboard_update_modifiers(
    WaylandKeyboardState& keyboard,
    std::uint32_t mods_depressed,
    std::uint32_t mods_latched,
    std::uint32_t mods_locked,
    std::uint32_t group) {
  keyboard.mods_depressed = mods_depressed;
  keyboard.mods_latched = mods_latched;
  keyboard.mods_locked = mods_locked;
  keyboard.layout_group = group;
  apply_modifier_state(keyboard);
}

KeyboardModifiers wayland_keyboard_modifiers(
    const WaylandKeyboardState& keyboard) {
  return modifiers_from_xkb_state(keyboard.state);
}

std::string wayland_keyboard_text_for_key(
    const WaylandKeyboardState& keyboard,
    std::uint32_t key) {
  if (keyboard.state == nullptr) {
    return {};
  }

  std::array<char, 64> buffer{};
  const auto length = xkb_state_key_get_utf8(
      keyboard.state,
      key + 8,
      buffer.data(),
      buffer.size());
  if (length <= 0 || static_cast<std::size_t>(length) >= buffer.size()) {
    return {};
  }
  return std::string(buffer.data(), static_cast<std::size_t>(length));
}

} // namespace cgpui
