#include "ui_internal.hpp"

namespace cgpui {

WindowRuntime::ActionDispatchTarget WindowRuntime::action_dispatch_target() {
  if (action_window_runtime_id_.has_value()) {
    const WindowRuntimeId runtime_id = *action_window_runtime_id_;
    if (const WindowRuntimeRecord* record =
            find_window_runtime_record(runtime_id);
        record != nullptr) {
      return ActionDispatchTarget{
          .focus_owner = record->input.keyboard_focus_element_owner,
          .view_id = record->event_route.has_value()
                         ? record->event_route->target_view_id
                         : record->root_view_id,
          // Looked up per handler: a handler may open or close windows.
          .context =
              [this, runtime_id] {
                const WindowRuntimeRecord* current =
                    find_window_runtime_record(runtime_id);
                return current != nullptr ? context_for_record(*current)
                                          : context();
              },
      };
    }
  }
  return ActionDispatchTarget{
      .focus_owner = keyboard_focus_element_owner_,
      .view_id = action_dispatch_view_id(),
      .context = [this] { return context(); },
  };
}

} // namespace cgpui
