#include "ui_internal.hpp"
#include "runtime_gesture_synthesis.hpp"

namespace cgpui {

EventResult WindowRuntime::dispatch_record_event_through_static_elements(
    WindowRuntimeRecord& record, View& view, const PlatformEvent& event) {
  const auto tree = *static_tree_for_record(record);
  const auto position = pointer_position_for(event);
  std::optional<ElementId> hit;
  if (position) {
    const auto id = tree.hit_test(*position);
    if (id.value != 0) hit = id;
  }
  if (record.event_route) {
    auto& route = *record.event_route;
    if (record.input.keyboard_focus_element_owner && is_keyboard_routed_event(event)) {
      route.target_element_id = record.input.keyboard_focus_element_owner;
    } else if (record.input.pointer_capture_owner && position) {
      apply_pointer_capture_owner_to_route(*record.input.pointer_capture_owner, route);
    } else if (hit) {
      route.target_element_id = hit;
    }
    route.element_ancestry.clear();
    const auto* target = route.target_element_id ? tree.get(*route.target_element_id) : nullptr;
    for (auto* node = target; node != nullptr; node = tree.get(node->parent_id)) {
      route.element_ancestry.push_back(node->id);
    }
    const auto enabled_target = target != nullptr && target->enabled
        ? route.target_element_id : std::optional<ElementId>{};
    const bool focusable = target != nullptr && target->enabled && target->focusable;
    synthesize_pointer_gesture_state(record.input, event, enabled_target);
    if (is_focus_activation_event(event) && focusable) {
      request_keyboard_focus_for_window(record.runtime_id, *enabled_target);
      if (!record.active) return EventResult::consumed_event();
    }
  }
  // Static nodes describe routing/accessibility; View owns event behavior/paint.
  return view.handle_event(event, context_for_record(record));
}

} // namespace cgpui
