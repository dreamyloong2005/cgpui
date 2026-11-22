#include "ui_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

void WindowRuntime::request_render_for_window(WindowRuntimeId runtime_id) {
  if (runtime_id == state_->windows.root_window_runtime_id_) {
    request_render();
    return;
  }
  request_paint_for_window(runtime_id);
}

void WindowRuntime::request_layout_for_window(WindowRuntimeId runtime_id) {
  if (runtime_id == state_->windows.root_window_runtime_id_) {
    request_layout();
    return;
  }
  request_paint_for_window(runtime_id);
}

void WindowRuntime::request_paint_for_window(WindowRuntimeId runtime_id) {
  if (runtime_id == state_->windows.root_window_runtime_id_) {
    request_paint();
    return;
  }
  WindowRuntimeRecord* record = find_window_runtime_record(runtime_id);
  if (record == nullptr || !record->active || record->window == nullptr ||
      record->redraw_scheduled || state_->core.should_quit_) return;
  record->redraw_scheduled = true;
  if (state_->windows.window_lifetime_->in_callback()) {
    defer([runtime_id](const auto& context) {
      auto* current = context.runtime.find_window_runtime_record(runtime_id);
      if (current != nullptr && current->active && current->window != nullptr && current->redraw_scheduled) {
        current->window->request_redraw();
      }
    });
    return;
  }
  record->window->request_redraw();
}

} // namespace cgpui
