#include "ui_internal.hpp"

namespace cgpui {

namespace {

int key_binding_context_rank(KeyBindingContextKind kind) {
  switch (kind) {
    case KeyBindingContextKind::focused_element:
      return 3;
    case KeyBindingContextKind::view:
      return 2;
    case KeyBindingContextKind::window:
      return 1;
    case KeyBindingContextKind::app:
      return 0;
  }
  return 0;
}

bool route_contains_view(const EventRoute& route, ViewId view_id) {
  if (route.target_view_id == view_id) {
    return true;
  }
  for (ViewId ancestor : route.view_ancestry) {
    if (ancestor == view_id) {
      return true;
    }
  }
  return false;
}

bool key_binding_matches_key(
    const KeyBinding& binding,
    const KeyboardKey& key) {
  return binding.key_code == key.key_code && binding.action == key.action &&
         modifiers_equal(binding.modifiers, key.modifiers);
}

} // namespace

KeyBindingContext KeyBindingContext::app() {
  return {};
}

KeyBindingContext KeyBindingContext::window() {
  return {.kind = KeyBindingContextKind::window};
}

KeyBindingContext KeyBindingContext::view(ViewId view_id) {
  return {
      .kind = KeyBindingContextKind::view,
      .view_id = view_id,
  };
}

KeyBindingContext KeyBindingContext::focused_element(ElementId element_id) {
  return {
      .kind = KeyBindingContextKind::focused_element,
      .element_id = element_id,
  };
}

bool WindowRuntime::key_binding_context_active(
    const KeyBindingContext& context) const {
  switch (context.kind) {
    case KeyBindingContextKind::app:
      return true;
    case KeyBindingContextKind::window:
      return true;
    case KeyBindingContextKind::view:
      if (!context.view_id.has_value()) {
        return false;
      }
      if (current_event_route_.has_value()) {
        return route_contains_view(*current_event_route_, *context.view_id);
      }
      return *context.view_id == root_view_id_;
    case KeyBindingContextKind::focused_element:
      return context.element_id.has_value() &&
             keyboard_focus_element_owner_ == context.element_id;
  }
  return false;
}

bool WindowRuntime::dispatch_key_binding_for_event(const KeyboardKey& key) {
  const KeyBinding* selected = nullptr;
  int selected_rank = -1;

  for (const KeyBinding& binding : key_bindings_) {
    if (!key_binding_matches_key(binding, key) ||
        !key_binding_context_active(binding.context)) {
      continue;
    }

    const int rank = key_binding_context_rank(binding.context.kind);
    if (selected == nullptr || rank > selected_rank) {
      selected = &binding;
      selected_rank = rank;
    }
  }

  if (selected == nullptr) {
    return false;
  }
  (void)dispatch_action(selected->action_name);
  return true;
}

} // namespace cgpui
