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
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::menu,
      .event_kind = EventKind::native_menu_command,
      .operation = command->source == NativeMenuCommandSource::accelerator
          ? "dispatch-native-menu-accelerator"
          : "dispatch-native-menu-command",
      .supported = true,
      .succeeded = dispatch.handled,
      .value_count = command->command_id,
  });
  finish_event_dispatch(dispatch.result);
  return true;
}

} // namespace cgpui
