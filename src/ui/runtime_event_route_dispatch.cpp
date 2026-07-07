#include "ui_internal.hpp"
#include "runtime_gesture_synthesis.hpp"

namespace cgpui {

std::optional<ViewId> WindowRuntime::action_dispatch_view_id() const {
  if (current_event_route_.has_value()) {
    return current_event_route_->target_view_id;
  }
  return root_view_id_;
}

ScrollState* WindowRuntime::scroll_state_for_route(const EventRoute& route) {
  if (!route.target_element_id.has_value()) {
    return nullptr;
  }

  const std::span<const ElementId> ancestry(route.element_ancestry);
  const auto route_ids =
      ancestry.empty()
          ? std::span<const ElementId>(&*route.target_element_id, 1)
          : ancestry;
  for (ElementId element_id : route_ids) {
    auto* scroll = dynamic_cast<ScrollElement*>(routed_element(element_id));
    if (scroll != nullptr && scroll->enabled()) {
      return scroll->state();
    }
  }

  return nullptr;
}

EventResult WindowRuntime::dispatch_routed_element_event(
    const PlatformEvent& event,
    const EventRoute& route) {
  if (!route.target_element_id.has_value()) {
    return EventResult::unhandled();
  }

  const ElementEventContext context{
      .target_element_id = *route.target_element_id,
      .dispatch_action =
          [this](std::string_view action_name) {
            const ActionDispatchResult dispatch =
                dispatch_action(std::string(action_name));
            return dispatch.result;
          },
  };
  const std::span<const ElementId> ancestry(route.element_ancestry);
  const auto route_ids =
      ancestry.empty()
          ? std::span<const ElementId>(&*route.target_element_id, 1)
          : ancestry;
  for (ElementId element_id : route_ids) {
    Element* element = routed_element(element_id);
    if (element == nullptr || !element->enabled()) {
      continue;
    }

    const EventResult result = element->handle_event(event, context);
    if (result.consumed || result.cancelled) {
      return result;
    }
  }

  if (should_dispatch_synthesized_click_event(input_, event, route)) {
    return dispatch_synthesized_click_event(
        event,
        route,
        context,
        [this](ElementId element_id) {
          return routed_element(element_id);
        });
  }

  return EventResult::unhandled();
}

} // namespace cgpui
