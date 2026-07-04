#include "ui_internal.hpp"

namespace cgpui {

namespace {

bool action_result_stops_bubbling(const EventResult& result) {
  return result.consumed || result.cancelled;
}

} // namespace

ActionDispatchResult WindowRuntime::dispatch_action(std::string name) {
  ActionDispatchResult dispatch{
      .name = std::move(name),
      .result = EventResult::unhandled()};

  const auto try_handler =
      [&](ActionScope scope,
          std::optional<ViewId> view_id,
          std::optional<ElementId> element_id,
          const ActionHandler& handler) {
        if (!action_registration_enabled(
                 dispatch.name,
                 scope,
                 view_id,
                 element_id)
                 .value_or(true)) {
          return false;
        }

        const EventResult result = handler(context());
        if (!action_result_stops_bubbling(result)) {
          return false;
        }

        dispatch.handled = true;
        dispatch.scope = scope;
        dispatch.view_id = view_id;
        dispatch.element_id = element_id;
        dispatch.result = result;
        last_action_dispatch_ = dispatch;
        return true;
      };

  if (keyboard_focus_element_owner_.has_value()) {
    const auto owner = focused_element_action_handlers_.find(
        keyboard_focus_element_owner_->value);
    if (owner != focused_element_action_handlers_.end()) {
      const auto handler = owner->second.find(dispatch.name);
      if (handler != owner->second.end() &&
          try_handler(
              ActionScope::focused_element,
              std::nullopt,
              keyboard_focus_element_owner_,
              handler->second)) {
        return dispatch;
      }
    }
  }

  if (const std::optional<ViewId> view_id = action_dispatch_view_id();
      view_id.has_value()) {
    const auto owner = view_action_handlers_.find(view_id->value);
    if (owner != view_action_handlers_.end()) {
      const auto handler = owner->second.find(dispatch.name);
      if (handler != owner->second.end() &&
          try_handler(ActionScope::view, view_id, std::nullopt, handler->second)) {
        return dispatch;
      }
    }
  }

  if (const auto handler = window_action_handlers_.find(dispatch.name);
      handler != window_action_handlers_.end() &&
      try_handler(ActionScope::window, std::nullopt, std::nullopt, handler->second)) {
    return dispatch;
  }

  if (const auto handler = action_handlers_.find(dispatch.name);
      handler != action_handlers_.end()) {
    (void)try_handler(
        ActionScope::app,
        std::nullopt,
        std::nullopt,
        handler->second);
  }

  last_action_dispatch_ = dispatch;
  return dispatch;
}

std::optional<ActionDispatchResult> WindowRuntime::last_action_dispatch()
    const {
  return last_action_dispatch_;
}

} // namespace cgpui
