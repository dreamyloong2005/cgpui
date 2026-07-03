#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::handle_redraw_for_record(
    WindowRuntimeRecord& record,
    View& view) {
  if (!record.active || record.window == nullptr || record.renderer == nullptr ||
      should_quit_) {
    return;
  }

  const WindowState state = record.window->state();
  const Size viewport_size = to_logical_pixels(
      state.framebuffer_size,
      state.scale);
  FrameStatistics frame_statistics;
  frame_statistics.render_pass_count = 1;

  ViewContext render_context = context_for_record(record);
  (void)view.render(render_context);

  last_render_record_ = RenderRecord{
      .sequence = ++render_sequence_,
      .view_id = record.root_view_id,
      .viewport_size = viewport_size,
  };
  if (after_render_callback_) {
    after_render_callback_(context_for_record(record), *last_render_record_);
  }

  auto result = render_view(
      *record.renderer,
      view,
      viewport_size,
      state.scale,
      &frame_statistics);
  if (!result) {
    fail_and_quit(result.error());
    return;
  }

  clear_invalidation();
  redraw_scheduled_ = false;
  deferred_redraw_request_ = false;
  frame_index_ += 1;
  frame_statistics.frame_index = frame_index_;
  if (last_render_record_.has_value()) {
    last_render_record_->statistics = frame_statistics;
  }
  last_frame_statistics_ = frame_statistics;
  if (after_frame_callback_) {
    after_frame_callback_(context_for_record(record));
  }
}

} // namespace cgpui
