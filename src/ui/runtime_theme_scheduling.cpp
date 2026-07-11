#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::request_theme_render(WindowRuntimeId runtime_id) {
  if (runtime_id == root_window_runtime_id_) {
    request_render();
    return;
  }
  WindowRuntimeRecord* record = find_window_runtime_record(runtime_id);
  if (record == nullptr || !record->active || record->window == nullptr ||
      record->redraw_scheduled) {
    return;
  }
  record->redraw_scheduled = true;
  record->window->request_redraw();
}

void WindowRuntime::request_all_theme_renders() {
  request_render();
  for (WindowRuntimeRecord& record : window_runtime_records_) {
    if (record.runtime_id != root_window_runtime_id_) {
      request_theme_render(record.runtime_id);
    }
  }
}

} // namespace cgpui
