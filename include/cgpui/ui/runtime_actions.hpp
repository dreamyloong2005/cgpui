#pragma once

#include "cgpui/ui/key_binding.hpp"
#include "cgpui/ui/runtime_callbacks.hpp"
#include "cgpui/ui/runtime_ids.hpp"

#include <optional>
#include <string>

namespace cgpui {

enum class ActionScope {
  app,
  window,
  view,
  focused_element,
};

enum class ActionRegistrationScope {
  general,
  app,
  window,
  view,
  focused_element,
};

struct ActionRegistrationOptions {
  bool enabled = true;
};

struct ActionRegistration {
  std::string name;
  ActionRegistrationScope registration_scope = ActionRegistrationScope::general;
  ActionScope dispatch_scope = ActionScope::app;
  bool enabled = true;
  std::optional<ViewId> view_id;
  std::optional<ElementId> element_id;
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
  std::string key_binding;
  std::optional<KeyBindingContext> key_context;
};

} // namespace cgpui
