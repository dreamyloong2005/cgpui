#pragma once

#include "cgpui/ui/runtime_callbacks.hpp"
#include "cgpui/ui/runtime_ids.hpp"
#include "cgpui/ui/text_edit_actions.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace cgpui {

enum class ActionScope {
  app,
  window,
  view,
  focused_element,
};

struct ActionDispatchResult {
  std::string name;
  bool handled = false;
  EventResult result;
  std::optional<ActionScope> scope;
  std::optional<ViewId> view_id;
  std::optional<ElementId> element_id;
};

struct CommandPaletteEntry {
  std::string action_name;
  std::string title;
  std::string group;
  ActionScope scope = ActionScope::app;
  bool enabled = true;
  std::optional<ViewId> view_id;
  std::optional<ElementId> element_id;
};

struct KeyBinding {
  std::uint32_t key_code = 0;
  KeyAction action = KeyAction::pressed;
  KeyboardModifiers modifiers;
  std::string action_name;
};

struct TextEditBinding {
  std::uint32_t key_code = 0;
  KeyAction action = KeyAction::pressed;
  KeyboardModifiers modifiers;
  TextEditAction edit_action = TextEditAction::move_previous;
};

} // namespace cgpui
