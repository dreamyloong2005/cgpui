#pragma once

#include "cgpui/ui/runtime.hpp"

#include <functional>
#include <optional>

namespace cgpui {

using RuntimeGestureElementResolver = std::function<Element*(ElementId)>;

void synthesize_pointer_gesture_state(
    ViewInputState& input,
    const PlatformEvent& event,
    std::optional<ElementId> enabled_target_element_id);

[[nodiscard]] bool should_dispatch_synthesized_click_event(
    const ViewInputState& input,
    const PlatformEvent& event,
    const EventRoute& route);

[[nodiscard]] EventResult dispatch_synthesized_click_event(
    const PlatformEvent& event,
    const EventRoute& route,
    ElementEventContext context,
    const RuntimeGestureElementResolver& resolve_element);

} // namespace cgpui
