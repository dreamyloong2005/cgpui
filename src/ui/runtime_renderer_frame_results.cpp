#include "ui_internal.hpp"

namespace cgpui {

Result<void> WindowRuntime::try_draw_frame() {
  if (renderer_ == nullptr || should_quit_) {
    return {};
  }

  FrameStatistics frame_statistics;
  frame_statistics.render_pass_count = 1;

  ViewContext render_context = context();
  AnyElement rendered = view_.render(render_context);
  std::optional<ElementId> rendered_root_id;
  if (rendered != nullptr) {
    auto tree = std::make_unique<ElementTree>();
    rendered_root_id = tree->set_root(std::move(rendered));
    set_element_tree(std::move(tree));
  }

  last_render_record_ = RenderRecord{
      .sequence = ++render_sequence_,
      .view_id = root_view_id_,
      .viewport_size = viewport_size_,
      .root_element_id = rendered_root_id,
  };
  if (after_render_callback_) {
    after_render_callback_(context(), *last_render_record_);
  }

  if (owned_element_tree_ != nullptr) {
    (void)owned_element_tree_->layout_root(LayoutInput{
        .constraints = {.max_size = viewport_size_},
        .scale = scale_,
    });
    frame_statistics.layout_pass_count += 1;
  }
  update_platform_accessibility_tree();
  apply_focused_text_ime_placement();

  auto result = render_view(
      *renderer_,
      view_,
      viewport_size_,
      scale_,
      &frame_statistics);
  if (!result) {
    return std::unexpected(result.error());
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
    after_frame_callback_(context());
  }
  return {};
}

Result<void> WindowRuntime::try_draw_frame_for_record(
    WindowRuntimeRecord& record,
    View& view) {
  if (!record.active || record.window == nullptr || record.renderer == nullptr ||
      should_quit_) {
    return {};
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
    return std::unexpected(result.error());
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
  return {};
}

} // namespace cgpui
