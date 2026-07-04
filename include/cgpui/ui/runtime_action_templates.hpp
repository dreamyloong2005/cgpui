#pragma once

#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/window_runtime.hpp"

#include <string>
#include <utility>

namespace cgpui {

template <Action T>
void WindowRuntime::register_action(ActionHandler handler) {
  register_action(std::string(action_name<T>()), std::move(handler));
}

template <Action T>
void WindowRuntime::register_app_action(ActionHandler handler) {
  register_app_action(std::string(action_name<T>()), std::move(handler));
}

template <Action T>
void WindowRuntime::register_window_action(ActionHandler handler) {
  register_window_action(std::string(action_name<T>()), std::move(handler));
}

template <Action T>
void WindowRuntime::register_view_action(
    ViewId view_id,
    ActionHandler handler) {
  register_view_action(
      view_id,
      std::string(action_name<T>()),
      std::move(handler));
}

template <Action T>
void WindowRuntime::register_focused_element_action(
    ElementId element_id,
    ActionHandler handler) {
  register_focused_element_action(
      element_id,
      std::string(action_name<T>()),
      std::move(handler));
}

template <Action T>
ActionDispatchResult WindowRuntime::dispatch_action() {
  return dispatch_action(std::string(action_name<T>()));
}

template <Action T>
void WindowRuntimeContext::register_action(ActionHandler handler) const {
  register_action(std::string(action_name<T>()), std::move(handler));
}

template <Action T>
void WindowRuntimeContext::register_app_action(ActionHandler handler) const {
  register_app_action(std::string(action_name<T>()), std::move(handler));
}

template <Action T>
void WindowRuntimeContext::register_window_action(ActionHandler handler) const {
  register_window_action(std::string(action_name<T>()), std::move(handler));
}

template <Action T>
void WindowRuntimeContext::register_view_action(ActionHandler handler) const {
  register_view_action(std::string(action_name<T>()), std::move(handler));
}

template <Action T>
void WindowRuntimeContext::register_view_action(
    ViewId view_id,
    ActionHandler handler) const {
  register_view_action(
      view_id,
      std::string(action_name<T>()),
      std::move(handler));
}

template <Action T>
void WindowRuntimeContext::register_focused_element_action(
    ElementId element_id,
    ActionHandler handler) const {
  register_focused_element_action(
      element_id,
      std::string(action_name<T>()),
      std::move(handler));
}

template <Action T>
ActionDispatchResult WindowRuntimeContext::dispatch_action() const {
  return dispatch_action(std::string(action_name<T>()));
}

} // namespace cgpui
