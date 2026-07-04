#include "ui_internal.hpp"

namespace cgpui {

namespace {

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

KeyBindingContext KeyBindingContext::disabled() const {
  KeyBindingContext context = *this;
  context.enabled = false;
  return context;
}

bool WindowRuntime::key_binding_context_active(
    const KeyBindingContext& context) const {
  if (!context.enabled) {
    return false;
  }

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

} // namespace cgpui
