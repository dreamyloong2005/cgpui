#include "wayland_application_internal.hpp"

namespace cgpui {

void WaylandApplication::handle_keyboard_keymap(
    void* data,
    wl_keyboard* keyboard,
    std::uint32_t format,
    std::int32_t fd,
    std::uint32_t size) {
  (void)keyboard;
  auto* app = static_cast<WaylandApplication*>(data);
  wayland_keyboard_load_keymap(app->keyboard_state_, format, fd, size);
}

void WaylandApplication::handle_keyboard_enter(
    void* data,
    wl_keyboard* keyboard,
    std::uint32_t serial,
    wl_surface* surface,
    wl_array* keys) {
  (void)keyboard;
  (void)serial;
  (void)keys;
  auto* app = static_cast<WaylandApplication*>(data);
  app->keyboard_window_ = app->find_window(surface);
  if (app->keyboard_window_ != nullptr) {
    wayland_window_focus_changed(*app->keyboard_window_, true);
  }
}

void WaylandApplication::handle_keyboard_leave(
    void* data,
    wl_keyboard* keyboard,
    std::uint32_t serial,
    wl_surface* surface) {
  (void)keyboard;
  (void)serial;
  auto* app = static_cast<WaylandApplication*>(data);
  if (app->keyboard_window_ != nullptr &&
      wayland_window_surface(*app->keyboard_window_) == surface) {
    wayland_window_focus_changed(*app->keyboard_window_, false);
    app->keyboard_window_ = nullptr;
  }
}

void WaylandApplication::handle_keyboard_key(
    void* data,
    wl_keyboard* keyboard,
    std::uint32_t serial,
    std::uint32_t time,
    std::uint32_t key,
    std::uint32_t state) {
  (void)keyboard;
  (void)serial;
  (void)time;
  auto* app = static_cast<WaylandApplication*>(data);
  if (app->keyboard_window_ != nullptr) {
    const auto modifiers = wayland_keyboard_modifiers(app->keyboard_state_);
    const auto action = state == WL_KEYBOARD_KEY_STATE_RELEASED
        ? KeyAction::released
        : KeyAction::pressed;
    wayland_window_keyboard_key(
        *app->keyboard_window_,
        key,
        action,
        modifiers);
    if (action == KeyAction::pressed) {
      auto text = wayland_keyboard_text_for_key(app->keyboard_state_, key);
      if (!text.empty()) {
        wayland_window_text_input(
            *app->keyboard_window_,
            std::move(text),
            modifiers);
      }
    }
  }
}

void WaylandApplication::handle_keyboard_modifiers(
    void* data,
    wl_keyboard* keyboard,
    std::uint32_t serial,
    std::uint32_t mods_depressed,
    std::uint32_t mods_latched,
    std::uint32_t mods_locked,
    std::uint32_t group) {
  (void)keyboard;
  (void)serial;
  auto* app = static_cast<WaylandApplication*>(data);
  wayland_keyboard_update_modifiers(
      app->keyboard_state_,
      mods_depressed,
      mods_latched,
      mods_locked,
      group);
}

void WaylandApplication::handle_keyboard_repeat_info(
    void* data,
    wl_keyboard* keyboard,
    std::int32_t rate,
    std::int32_t delay) {
  (void)data;
  (void)keyboard;
  (void)rate;
  (void)delay;
}

} // namespace cgpui
