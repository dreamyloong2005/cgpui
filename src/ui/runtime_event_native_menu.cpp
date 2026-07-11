#include "ui_internal.hpp"

namespace cgpui {

bool WindowRuntime::handle_native_menu_command_event(
    const PlatformEvent& event,
    ViewId target_view_id) {
  const auto* command = std::get_if<NativeMenuCommand>(&event);
  if (command == nullptr) return false;
  current_event_route_ = EventRouter::route_to_root(event, target_view_id);
  refresh_route_ancestry(*current_event_route_);
  const ActionDispatchResult dispatch = dispatch_action(command->action_name);
  finish_event_dispatch(dispatch.result);
  return true;
}

} // namespace cgpui
