#include "ui_internal.hpp"
#include "runtime_window_store_internal.hpp"

namespace cgpui {

void WindowRuntime::request_theme_render(WindowRuntimeId runtime_id) {
  request_render_for_window(runtime_id);
}

void WindowRuntime::request_all_theme_renders() {
  request_render();
  for (WindowRuntimeRecord& record : state_->windows.window_runtime_records_->records()) {
    if (record.runtime_id != state_->windows.root_window_runtime_id_) {
      request_theme_render(record.runtime_id);
    }
  }
}

} // namespace cgpui
