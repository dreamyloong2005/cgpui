#include "ui_internal.hpp"

#include <utility>

namespace cgpui {

ViewId WindowRuntimeContext::allocate_view_id() const {
  return runtime.allocate_view_id();
}

std::optional<EventRoute> WindowRuntimeContext::current_event_route() const {
  return event_route;
}

ViewInputState WindowRuntimeContext::input_state() const {
  return input;
}

bool WindowRuntimeContext::is_view_id_allocated(ViewId view_id) const {
  return runtime.is_view_id_allocated(view_id);
}

std::optional<ViewId> WindowRuntimeContext::upgrade_view(WeakView view) const {
  return runtime.upgrade_view(view);
}

void WindowRuntimeContext::capture_pointer(PointerCaptureOwner owner) const {
  runtime.capture_pointer(owner);
}

void WindowRuntimeContext::capture_pointer(ElementId element_id) const {
  runtime.capture_pointer(PointerCaptureOwner::element(element_id));
}

void WindowRuntimeContext::release_pointer(PointerCaptureOwner owner) const {
  runtime.release_pointer(owner);
}

void WindowRuntimeContext::release_pointer(ElementId element_id) const {
  runtime.release_pointer(PointerCaptureOwner::element(element_id));
}

void WindowRuntimeContext::request_keyboard_focus() const {
  runtime.request_keyboard_focus();
}

void WindowRuntimeContext::request_keyboard_focus(ViewId view_id) const {
  runtime.request_keyboard_focus(view_id);
}

void WindowRuntimeContext::request_keyboard_focus(ElementId element_id) const {
  runtime.request_keyboard_focus(element_id);
}

void WindowRuntimeContext::focus(ElementId element_id) const {
  runtime.request_keyboard_focus(element_id);
}

void WindowRuntimeContext::release_keyboard_focus() const {
  runtime.release_keyboard_focus();
}

void WindowRuntimeContext::release_keyboard_focus(ViewId view_id) const {
  runtime.release_keyboard_focus(view_id);
}

void WindowRuntimeContext::release_keyboard_focus(ElementId element_id) const {
  runtime.release_keyboard_focus(element_id);
}

void WindowRuntimeContext::blur(ElementId element_id) const {
  runtime.release_keyboard_focus(element_id);
}

FocusHandle WindowRuntimeContext::focus_handle(ElementId element_id) const {
  return runtime.focus_handle(element_id);
}

void WindowRuntimeContext::set_element_tree(
    std::unique_ptr<ElementTree> tree) const {
  runtime.set_element_tree(std::move(tree));
}

} // namespace cgpui
