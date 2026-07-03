#include "ui_internal.hpp"

namespace cgpui {

void AppContext::register_command_palette_entry(
    CommandPaletteEntry entry) const {
  runtime.register_command_palette_entry(std::move(entry));
}

std::span<const CommandPaletteEntry> AppContext::command_palette_entries()
    const {
  return runtime.command_palette_entries();
}

std::vector<CommandPaletteEntry> AppContext::command_palette_entries_for_group(
    std::string_view group) const {
  return runtime.command_palette_entries_for_group(group);
}

ActionDispatchResult AppContext::dispatch_command_palette_entry(
    const CommandPaletteEntry& entry) const {
  return runtime.dispatch_command_palette_entry(entry);
}

ActionDispatchResult AppContext::dispatch_command_palette_action(
    std::string action_name) const {
  return runtime.dispatch_command_palette_action(std::move(action_name));
}

} // namespace cgpui
