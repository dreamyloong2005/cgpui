#include "ui_internal.hpp"

namespace cgpui {

ActionDispatchResult WindowRuntime::dispatch_action(std::string name) {
  ActionDispatchResult dispatch{
      .name = std::move(name),
      .result = EventResult::unhandled()};

  if (keyboard_focus_element_owner_.has_value()) {
    const auto owner = focused_element_action_handlers_.find(
        keyboard_focus_element_owner_->value);
    if (owner != focused_element_action_handlers_.end()) {
      const auto handler = owner->second.find(dispatch.name);
      if (handler != owner->second.end() &&
          action_registration_enabled(
              dispatch.name,
              ActionScope::focused_element,
              std::nullopt,
              keyboard_focus_element_owner_)
              .value_or(true)) {
        dispatch.handled = true;
        dispatch.scope = ActionScope::focused_element;
        dispatch.element_id = keyboard_focus_element_owner_;
        dispatch.result = handler->second(context());
        last_action_dispatch_ = dispatch;
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
          action_registration_enabled(
              dispatch.name,
              ActionScope::view,
              view_id,
              std::nullopt)
              .value_or(true)) {
        dispatch.handled = true;
        dispatch.scope = ActionScope::view;
        dispatch.view_id = view_id;
        dispatch.result = handler->second(context());
        last_action_dispatch_ = dispatch;
        return dispatch;
      }
    }
  }

  if (const auto handler = window_action_handlers_.find(dispatch.name);
      handler != window_action_handlers_.end() &&
      action_registration_enabled(
          dispatch.name,
          ActionScope::window,
          std::nullopt,
          std::nullopt)
          .value_or(true)) {
    dispatch.handled = true;
    dispatch.scope = ActionScope::window;
    dispatch.result = handler->second(context());
    last_action_dispatch_ = dispatch;
    return dispatch;
  }

  if (const auto handler = action_handlers_.find(dispatch.name);
      handler != action_handlers_.end() &&
      action_registration_enabled(
          dispatch.name,
          ActionScope::app,
          std::nullopt,
          std::nullopt)
          .value_or(true)) {
    dispatch.handled = true;
    dispatch.scope = ActionScope::app;
    dispatch.result = handler->second(context());
  }

  last_action_dispatch_ = dispatch;
  return dispatch;
}

std::optional<ActionDispatchResult> WindowRuntime::last_action_dispatch()
    const {
  return last_action_dispatch_;
}

} // namespace cgpui
