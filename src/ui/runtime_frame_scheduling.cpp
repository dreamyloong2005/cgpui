#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::begin_frame_scheduling() {
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
  if (request_next_frame) {
    schedule_redraw();
  }
}

void WindowRuntime::abort_frame_scheduling() {
  rendering_frame_ = false;
  redraw_scheduled_ = false;
  deferred_redraw_request_ = false;
  next_frame_redraw_requested_ = false;
}

} // namespace cgpui
