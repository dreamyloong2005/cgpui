#include "ui_internal.hpp"

namespace cgpui {

AnyElement View::render(ViewContext& context) {
  (void)context;
  return {};
}

EventResult View::handle_event(
    const PlatformEvent& event,
    const WindowRuntimeContext& context) {
  (void)event;
  (void)context;
  return EventResult::unhandled();
}

EventRoute EventRouter::route_to_root(
    const PlatformEvent& event,
    ViewId root_view_id) {
  return EventRoute{
      .target_view_id = root_view_id,
      .view_ancestry = {root_view_id},
      .event_kind = event_kind_for(event)};
}


void FocusHandle::request(WindowRuntime& runtime) const {
  runtime.request_keyboard_focus(id_);
}

void FocusHandle::request(const WindowRuntimeContext& context) const {
  context.runtime.request_keyboard_focus(id_);
}

void FocusHandle::release(WindowRuntime& runtime) const {
  runtime.release_keyboard_focus(id_);
}

void FocusHandle::release(const WindowRuntimeContext& context) const {
  context.runtime.release_keyboard_focus(id_);
}

bool FocusHandle::contains(const ViewInputState& input) const {
  return id_.value != 0 && input.keyboard_focus_element_owner == id_;
}

bool FocusHandle::contains(const WindowRuntime& runtime) const {
  return contains(runtime.input_state());
}

bool FocusHandle::contains(const WindowRuntimeContext& context) const {
  return contains(context.input_state());
}

bool FocusHandle::focused(const ViewInputState& input) const {
  return contains(input);
}

bool FocusHandle::focused(const WindowRuntime& runtime) const {
  return contains(runtime);
}

bool FocusHandle::focused(const WindowRuntimeContext& context) const {
  return contains(context);
}


} // namespace cgpui
