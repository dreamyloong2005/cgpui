#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::register_action(std::string name, ActionHandler handler) {
  register_action(std::move(name), std::move(handler), {});
}

void WindowRuntime::register_action(
    std::string name,
    ActionHandler handler,
    ActionRegistrationOptions options) {
  if (!name.empty() && handler) {
    upsert_action_registration(ActionRegistration{
        .name = name,
        .registration_scope = ActionRegistrationScope::general,
        .dispatch_scope = ActionScope::app,
        .enabled = options.enabled});
    action_handlers_[std::move(name)] = std::move(handler);
  }
}

void WindowRuntime::register_app_action(std::string name, ActionHandler handler) {
  register_app_action(std::move(name), std::move(handler), {});
}

void WindowRuntime::register_app_action(
    std::string name,
    ActionHandler handler,
    ActionRegistrationOptions options) {
  if (!name.empty() && handler) {
    upsert_action_registration(ActionRegistration{
        .name = name,
        .registration_scope = ActionRegistrationScope::app,
        .dispatch_scope = ActionScope::app,
        .enabled = options.enabled});
    action_handlers_[std::move(name)] = std::move(handler);
  }
}

void WindowRuntime::register_window_action(std::string name, ActionHandler handler) {
  register_window_action(std::move(name), std::move(handler), {});
}

void WindowRuntime::register_window_action(
    std::string name,
    ActionHandler handler,
    ActionRegistrationOptions options) {
  if (!name.empty() && handler) {
    upsert_action_registration(ActionRegistration{
        .name = name,
        .registration_scope = ActionRegistrationScope::window,
        .dispatch_scope = ActionScope::window,
        .enabled = options.enabled});
    window_action_handlers_[std::move(name)] = std::move(handler);
  }
}

void WindowRuntime::register_view_action(
    ViewId view_id,
    std::string name,
    ActionHandler handler) {
  register_view_action(view_id, std::move(name), std::move(handler), {});
}

void WindowRuntime::register_view_action(
    ViewId view_id,
    std::string name,
    ActionHandler handler,
    ActionRegistrationOptions options) {
  if (view_id.value != 0 && !name.empty() && handler) {
    upsert_action_registration(ActionRegistration{
        .name = name,
        .registration_scope = ActionRegistrationScope::view,
        .dispatch_scope = ActionScope::view,
        .enabled = options.enabled,
        .view_id = view_id});
    view_action_handlers_[view_id.value][std::move(name)] =
        std::move(handler);
  }
}

void WindowRuntime::register_focused_element_action(
    ElementId element_id,
    std::string name,
    ActionHandler handler) {
  register_focused_element_action(
      element_id,
      std::move(name),
      std::move(handler),
      {});
}

void WindowRuntime::register_focused_element_action(
    ElementId element_id,
    std::string name,
    ActionHandler handler,
    ActionRegistrationOptions options) {
  if (element_id.value != 0 && !name.empty() && handler) {
    upsert_action_registration(ActionRegistration{
        .name = name,
        .registration_scope = ActionRegistrationScope::focused_element,
        .dispatch_scope = ActionScope::focused_element,
        .enabled = options.enabled,
        .element_id = element_id});
    focused_element_action_handlers_[element_id.value][std::move(name)] =
        std::move(handler);
  }
}

} // namespace cgpui
