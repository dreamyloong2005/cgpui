#include "ui_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"
#include "runtime_window_store_internal.hpp"

namespace cgpui {

void WindowRuntime::deactivate_native_additional_windows() {
  {
    const RuntimeWindowCallbackScope shutdown_scope(*this);
    for (WindowRuntimeRecord& record : state_->windows.window_runtime_records_->records()) {
      if (record.runtime_id == state_->windows.root_window_runtime_id_) continue;
      state_->windows.surface_resizes_.clear(record.runtime_id);
      state_->scheduling.frame_readiness_.clear(*this, record.runtime_id);
      Renderer* renderer = record.renderer;
      record.owns_renderer = false;
      record.window = nullptr;
      record.renderer = nullptr;
      record.active = false;
      cancel_window_timers(record.runtime_id);
      record.redraw_scheduled = false;
      record.input = {};
      reset_event_state_for_record(record);
      reset_accessibility_state_for_record(record);
      state_->windows.window_lifetime_->retire(renderer);
    }
    // Preserve view/metadata for another run(), but detach every surface before
    // a renderer destructor can reenter popup closing or record reclamation.
    for (auto& window : state_->windows.native_additional_windows_) {
      state_->windows.retired_native_windows_.push_back(std::move(window));
    }
    state_->windows.native_additional_windows_.clear();
  }
  collect_retired_native_windows();
}

} // namespace cgpui
