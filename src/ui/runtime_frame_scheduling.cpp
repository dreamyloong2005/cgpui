#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::set_root_redraw_scheduled(bool scheduled) {
  if (WindowRuntimeRecord* record =
          find_window_runtime_record(root_window_runtime_id_);
      record != nullptr) {
    record->redraw_scheduled = scheduled;
  }
}

void WindowRuntime::begin_frame_scheduling() {
  set_root_redraw_scheduled(true);
  invalidation_state_ = {};
  deferred_redraw_request_ = false;
  rendering_frame_ = true;
  next_frame_redraw_requested_ = false;
}

void WindowRuntime::complete_frame_scheduling() {
  rendering_frame_ = false;
  redraw_scheduled_ = false;
  deferred_redraw_request_ = false;
  const bool request_next_frame = next_frame_redraw_requested_;
  next_frame_redraw_requested_ = false;
  set_root_redraw_scheduled(false);
  if (request_next_frame) {
    schedule_redraw();
  }
}

void WindowRuntime::abort_frame_scheduling() {
  rendering_frame_ = false;
  redraw_scheduled_ = false;
  deferred_redraw_request_ = false;
  next_frame_redraw_requested_ = false;
  set_root_redraw_scheduled(false);
}

} // namespace cgpui
