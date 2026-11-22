#pragma once

#include "cgpui/ui/window_runtime.hpp"
#include "runtime_state_actions_types_internal.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeActionState {
  std::unordered_map<std::string, ActionHandler> action_handlers_;
  std::unordered_map<std::string, ActionHandler> window_action_handlers_;
  std::unordered_map<
      std::uint64_t,
      std::unordered_map<std::string, ActionHandler>>
      view_action_handlers_;
  std::unordered_map<
      std::uint64_t,
      std::unordered_map<std::string, ActionHandler>>
      focused_element_action_handlers_;
  std::vector<ActionRegistration> action_registrations_;
  std::vector<CommandPaletteEntry> command_palette_entries_;
  std::vector<KeyBinding> key_bindings_;
  std::vector<KeyBindingChord> pending_key_binding_sequence_;
  #include "runtime_window_action_dispatch_internal.hpp"
};

} // namespace cgpui
