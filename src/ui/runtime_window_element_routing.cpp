#include "ui_internal.hpp"

namespace cgpui {
namespace {

[[nodiscard]] std::vector<ElementId> tree_ancestry(
    const ElementTree& tree,
    ElementId element_id) {
  std::vector<ElementId> ancestry;
  std::optional<ElementId> current = element_id;
  while (current.has_value() && current->value != 0 &&
         tree.get(*current) != nullptr) {
    ancestry.push_back(*current);
    current = tree.parent(*current);
  }
  return ancestry;
}

} // namespace

void WindowRuntime::update_record_hover_cursor(
    WindowRuntimeRecord& record,
    const PlatformEvent& event,
    std::optional<ElementId> hit) {
  const bool exited = std::holds_alternative<PointerExited>(event);
  if (!exited && !std::holds_alternative<PointerMoved>(event)) {
    return;
  }
  CursorShape cursor = CursorShape::default_arrow;
  record.input.hovered_element_id = exited ? std::nullopt : hit;
  const ElementTree* tree = element_tree_for_record(record);
  if (!exited && hit.has_value() && tree != nullptr) {
    const Element* element = tree->get(*hit);
    const WindowElementHost& host = window_element_hosts_[record.runtime_id.value];
    if (const auto found = host.cursors.find(hit->value);
        found != host.cursors.end() && (element == nullptr || element->enabled())) {
      cursor = found->second;
    }
  }
  if (record.input.cursor_shape != cursor && record.window != nullptr) {
    record.window->set_cursor(cursor);
  }
  record.input.cursor_shape = cursor;
}

// Hit test, cursor, target (focus owner for keys, capture owner while
// captured, else the hit element), focus on a left press, then the element
// ancestry; the view gets the event unless an element consumed it.
EventResult WindowRuntime::dispatch_record_event_through_elements(
    WindowRuntimeRecord& record,
    View& view,
    const PlatformEvent& event) {
  ElementTree* tree = element_tree_for_record(record);
  const std::optional<Point> position = pointer_position_for(event);
  std::optional<ElementId> hit;
  if (tree != nullptr && position.has_value()) {
    if (const ElementId id = tree->hit_test_root(*position); id.value != 0) {
      hit = id;
    }
  }
  update_record_hover_cursor(record, event, hit);
  if (tree != nullptr && record.event_route.has_value()) {
    EventRoute& route = *record.event_route;
    if (record.input.keyboard_focus_element_owner.has_value() &&
        is_keyboard_routed_event(event)) {
      route.target_element_id = record.input.keyboard_focus_element_owner;
    } else if (record.input.pointer_capture_owner.has_value() &&
               position.has_value()) {
      apply_pointer_capture_owner_to_route(
          *record.input.pointer_capture_owner, route);
    } else if (hit.has_value()) {
      route.target_element_id = hit;
    }
    route.element_ancestry.clear();
    if (route.target_element_id.has_value()) {
      route.element_ancestry = tree_ancestry(*tree, *route.target_element_id);
    }
    if (is_focus_activation_event(event) && route.target_element_id.has_value()) {
      Element* target = tree->get(*route.target_element_id);
      if (target != nullptr && target->enabled() && target->focusable()) {
        request_keyboard_focus_for_window(record.runtime_id, *route.target_element_id);
        target->focus(ElementFocusContext{.element_id = *route.target_element_id});
      }
    }
    const ElementEventContext element_context{
        .target_element_id = route.target_element_id.value_or(ElementId{}),
        .dispatch_action =
            [this](std::string_view action_name) {
              return dispatch_action(std::string(action_name)).result;
            },
    };
    for (const ElementId element_id : route.element_ancestry) {
      Element* element = tree->get(element_id);
      if (element == nullptr || !element->enabled()) continue;
      const EventResult result = element->handle_event(event, element_context);
      if (result.consumed || result.cancelled) return result;
    }
  }
  return view.handle_event(event, context_for_record(record));
}

} // namespace cgpui
