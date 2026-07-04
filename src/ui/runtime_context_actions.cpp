#include "ui_internal.hpp"

#include <utility>

namespace cgpui {

void WindowRuntimeContext::register_action(
    std::string name,
    ActionHandler handler) const {
  register_action(std::move(name), std::move(handler), {});
}

void WindowRuntimeContext::register_action(
    std::string name,
    ActionHandler handler,
    ActionRegistrationOptions options) const {
  runtime.register_action(std::move(name), std::move(handler), options);
}

void WindowRuntimeContext::register_app_action(
    std::string name,
    ActionHandler handler) const {
  register_app_action(std::move(name), std::move(handler), {});
}

void WindowRuntimeContext::register_app_action(
    std::string name,
    ActionHandler handler,
    ActionRegistrationOptions options) const {
  runtime.register_app_action(std::move(name), std::move(handler), options);
}

void WindowRuntimeContext::register_window_action(
    std::string name,
    ActionHandler handler) const {
  register_window_action(std::move(name), std::move(handler), {});
}

void WindowRuntimeContext::register_window_action(
    std::string name,
    ActionHandler handler,
    ActionRegistrationOptions options) const {
  runtime.register_window_action(std::move(name), std::move(handler), options);
}

void WindowRuntimeContext::register_view_action(
    std::string name,
    ActionHandler handler) const {
  register_view_action(std::move(name), std::move(handler), {});
}

void WindowRuntimeContext::register_view_action(
    std::string name,
    ActionHandler handler,
    ActionRegistrationOptions options) const {
  runtime.register_view_action(
      view_id,
      std::move(name),
      std::move(handler),
      options);
}

void WindowRuntimeContext::register_view_action(
    ViewId target_view_id,
    std::string name,
    ActionHandler handler) const {
  register_view_action(
      target_view_id,
      std::move(name),
      std::move(handler),
      {});
}

void WindowRuntimeContext::register_view_action(
    ViewId target_view_id,
    std::string name,
    ActionHandler handler,
    ActionRegistrationOptions options) const {
  runtime.register_view_action(
      target_view_id,
      std::move(name),
      std::move(handler),
      options);
}

void WindowRuntimeContext::register_focused_element_action(
    ElementId element_id,
    std::string name,
    ActionHandler handler) const {
  register_focused_element_action(
      element_id,
      std::move(name),
      std::move(handler),
      {});
}

void WindowRuntimeContext::register_focused_element_action(
    ElementId element_id,
    std::string name,
    ActionHandler handler,
    ActionRegistrationOptions options) const {
  runtime.register_focused_element_action(
      element_id,
      std::move(name),
      std::move(handler),
      options);
}

ActionDispatchResult WindowRuntimeContext::dispatch_action(
    std::string name) const {
  return runtime.dispatch_action(std::move(name));
}

std::optional<ActionDispatchResult> WindowRuntimeContext::last_action_dispatch()
    const {
  return runtime.last_action_dispatch();
}

std::span<const ActionRegistration> WindowRuntimeContext::action_registrations()
    const {
  return runtime.action_registrations();
}

std::vector<ActionRegistration>
WindowRuntimeContext::action_registrations_for_scope(
    ActionRegistrationScope scope) const {
  return runtime.action_registrations_for_scope(scope);
}

std::vector<ActionRegistration>
WindowRuntimeContext::action_registrations_for_enabled(bool enabled) const {
  return runtime.action_registrations_for_enabled(enabled);
}

void WindowRuntimeContext::register_command_palette_entry(
    CommandPaletteEntry entry) const {
  runtime.register_command_palette_entry(std::move(entry));
}

std::span<const CommandPaletteEntry>
WindowRuntimeContext::command_palette_entries() const {
  return runtime.command_palette_entries();
}

std::vector<CommandPaletteEntry>
WindowRuntimeContext::command_palette_entries_for_group(
    std::string_view group) const {
  return runtime.command_palette_entries_for_group(group);
}

ActionDispatchResult WindowRuntimeContext::dispatch_command_palette_entry(
    const CommandPaletteEntry& entry) const {
  return runtime.dispatch_command_palette_entry(entry);
}

ActionDispatchResult WindowRuntimeContext::dispatch_command_palette_action(
    std::string action_name) const {
  return runtime.dispatch_command_palette_action(std::move(action_name));
}

void WindowRuntimeContext::bind_key(KeyBinding binding) const {
  runtime.bind_key(std::move(binding));
}

void WindowRuntimeContext::bind_text_edit_action(
    TextEditBinding binding) const {
  runtime.bind_text_edit_action(std::move(binding));
}

void WindowRuntimeContext::set_element_cursor(
    ElementId element_id,
    CursorShape cursor_shape) const {
  runtime.set_element_cursor(element_id, cursor_shape);
}

} // namespace cgpui
