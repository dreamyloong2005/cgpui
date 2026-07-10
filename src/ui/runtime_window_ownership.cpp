#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::activate_pending_native_windows() {
  for (WindowRuntimeRecord& record : window_runtime_records_) {
    if (record.runtime_id == root_window_runtime_id_ || record.active ||
        record.native_window_error.has_value()) {
      continue;
    }
    activate_native_window_for_record(record);
  }
}

} // namespace cgpui
