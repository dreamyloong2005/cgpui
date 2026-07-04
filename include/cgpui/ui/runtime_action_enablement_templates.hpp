#pragma once

#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/window_runtime.hpp"

#include <string>
#include <utility>

namespace cgpui {

template <Action T>
void WindowRuntime::register_action(
    ActionHandler handler,
    ActionRegistrationOptions options) {
  register_action(std::string(action_name<T>()), std::move(handler), options);
}

template <Action T>
void WindowRuntime::register_app_action(
    ActionHandler handler,
    ActionRegistrationOptions options) {
  register_app_action(std::string(action_name<T>()), std::move(handler), options);
}

template <Action T>
void WindowRuntime::register_window_action(
    ActionHandler handler,
    ActionRegistrationOptions options) {
  register_window_action(std::string(action_name<T>()), std::move(handler), options);
}

template <Action T>
void WindowRuntime::register_view_action(
    ViewId view_id,
    ActionHandler handler,
    ActionRegistrationOptions options) {
  register_view_action(view_id, std::string(action_name<T>()), std::move(handler), options);
}

template <Action T>
void WindowRuntime::register_focused_element_action(
    ElementId element_id,
    ActionHandler handler,
    ActionRegistrationOptions options) {
  register_focused_element_action(element_id, std::string(action_name<T>()), std::move(handler), options);
}

template <Action T>
void WindowRuntimeContext::register_action(
    ActionHandler handler,
    ActionRegistrationOptions options) const {
  register_action(std::string(action_name<T>()), std::move(handler), options);
}

template <Action T>
void WindowRuntimeContext::register_app_action(
    ActionHandler handler,
    ActionRegistrationOptions options) const {
  register_app_action(std::string(action_name<T>()), std::move(handler), options);
}

template <Action T>
void WindowRuntimeContext::register_window_action(
    ActionHandler handler,
    ActionRegistrationOptions options) const {
  register_window_action(std::string(action_name<T>()), std::move(handler), options);
}

template <Action T>
void WindowRuntimeContext::register_view_action(
    ActionHandler handler,
    ActionRegistrationOptions options) const {
  register_view_action(std::string(action_name<T>()), std::move(handler), options);
}

template <Action T>
void WindowRuntimeContext::register_view_action(
    ViewId view_id,
    ActionHandler handler,
    ActionRegistrationOptions options) const {
  register_view_action(view_id, std::string(action_name<T>()), std::move(handler), options);
}

template <Action T>
void WindowRuntimeContext::register_focused_element_action(
    ElementId element_id,
    ActionHandler handler,
    ActionRegistrationOptions options) const {
  register_focused_element_action(element_id, std::string(action_name<T>()), std::move(handler), options);
}

} // namespace cgpui
