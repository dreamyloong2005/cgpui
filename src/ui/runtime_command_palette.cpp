#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::register_command_palette_entry(CommandPaletteEntry entry) {
  if (!entry.action_name.empty() && !entry.title.empty()) {
    std::optional<KeyBinding> binding = command_palette_key_binding(entry);
    command_palette_entries_.push_back(std::move(entry));
    if (binding.has_value()) {
      bind_key(std::move(*binding));
    }
  }
}

std::span<const CommandPaletteEntry> WindowRuntime::command_palette_entries()
    const {
  return command_palette_entries_;
}

std::vector<CommandPaletteEntry>
WindowRuntime::command_palette_entries_for_group(std::string_view group) const {
  std::vector<CommandPaletteEntry> entries;
  for (const CommandPaletteEntry& entry : command_palette_entries_) {
    if (entry.group == group) {
      entries.push_back(entry);
    }
  }
  return entries;
}

ActionDispatchResult WindowRuntime::dispatch_command_palette_entry(
    const CommandPaletteEntry& entry) {
  if (!entry.enabled || entry.action_name.empty()) {
    ActionDispatchResult dispatch{
        .name = entry.action_name,
        .result = EventResult::unhandled()};
    last_action_dispatch_ = dispatch;
    return dispatch;
  }

  return dispatch_action(entry.action_name);
}

ActionDispatchResult WindowRuntime::dispatch_command_palette_action(
    std::string action_name) {
  for (const CommandPaletteEntry& entry : command_palette_entries_) {
    if (entry.action_name == action_name) {
      return dispatch_command_palette_entry(entry);
    }
  }

  ActionDispatchResult dispatch{
      .name = std::move(action_name),
      .result = EventResult::unhandled()};
  last_action_dispatch_ = dispatch;
  return dispatch;
}

} // namespace cgpui
