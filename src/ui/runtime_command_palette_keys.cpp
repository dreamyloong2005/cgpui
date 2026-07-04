#include "ui_internal.hpp"

namespace cgpui {
namespace {

std::optional<KeyBindingContext> command_palette_key_context(
    const CommandPaletteEntry& entry) {
  if (entry.key_context.has_value()) {
    return entry.key_context;
  }

  switch (entry.scope) {
    case ActionScope::app:
      return KeyBindingContext::app();
    case ActionScope::window:
      return KeyBindingContext::window();
    case ActionScope::view:
      if (entry.view_id.has_value()) {
        return KeyBindingContext::view(*entry.view_id);
      }
      return std::nullopt;
    case ActionScope::focused_element:
      if (entry.element_id.has_value()) {
        return KeyBindingContext::focused_element(*entry.element_id);
      }
      return std::nullopt;
  }

  return std::nullopt;
}

} // namespace

std::optional<KeyBinding> WindowRuntime::command_palette_key_binding(
    const CommandPaletteEntry& entry) const {
  if (!entry.enabled || entry.key_binding.empty()) {
    return std::nullopt;
  }

  std::optional<KeyBinding> binding =
      parse_key_binding(entry.key_binding, entry.action_name);
  std::optional<KeyBindingContext> context =
      command_palette_key_context(entry);
  if (!binding.has_value() || !context.has_value()) {
    return std::nullopt;
  }

  binding->context = *context;
  return binding;
}

} // namespace cgpui
