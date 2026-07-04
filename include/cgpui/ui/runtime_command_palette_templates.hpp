#pragma once

#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/window_runtime.hpp"

#include <string>
#include <utility>

namespace cgpui {

template <Action T>
[[nodiscard]] CommandPaletteEntry command_palette_entry(
    CommandPaletteEntry entry) {
  entry.action_name = std::string(action_name<T>());
  return entry;
}

template <Action T>
void WindowRuntime::register_command_palette_entry(CommandPaletteEntry entry) {
  register_command_palette_entry(command_palette_entry<T>(std::move(entry)));
}

template <Action T>
void WindowRuntimeContext::register_command_palette_entry(
    CommandPaletteEntry entry) const {
  register_command_palette_entry(command_palette_entry<T>(std::move(entry)));
}

template <Action T>
void AppContext::register_command_palette_entry(
    CommandPaletteEntry entry) const {
  runtime.register_command_palette_entry<T>(std::move(entry));
}

} // namespace cgpui
