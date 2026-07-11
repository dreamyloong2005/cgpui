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

void WindowRuntime::collect_retired_native_windows() {
  retired_native_windows_.clear();
  reclaim_closed_window_records();
}

} // namespace cgpui
