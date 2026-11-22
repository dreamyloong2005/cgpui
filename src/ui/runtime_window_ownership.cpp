#include "ui_internal.hpp"
#include "runtime_window_store_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

void WindowRuntime::activate_pending_native_windows() {
  for (WindowRuntimeRecord& record : state_->windows.window_runtime_records_->records()) {
    if (record.runtime_id == state_->windows.root_window_runtime_id_ || record.active ||
        record.native_window_error.has_value()) {
      continue;
    }
    activate_native_window_for_record(record);
  }
}

void WindowRuntime::collect_retired_native_windows() {
  if (!state_->windows.window_lifetime_->begin_collection()) return;
  state_->windows.window_lifetime_->release_retired(state_->windows.retired_native_windows_);
  reclaim_closed_window_records();
  state_->windows.window_lifetime_->end_collection();
}

} // namespace cgpui
