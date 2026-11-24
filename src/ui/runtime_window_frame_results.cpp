#include "ui_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

Result<void> WindowRuntime::try_draw_frame_for_record(
    WindowRuntimeRecord& record,
    View& view) {
  if (!record.active || record.window == nullptr || record.renderer == nullptr ||
      state_->core.should_quit_) {
    return {};
  }
  const RuntimeWindowCallbackScope callback_scope(*this);
  record.redraw_scheduled = false;
  if (auto resized = state_->windows.surface_resizes_.retry(record.runtime_id, *record.renderer); !resized)
    return resized;
  if (!record.active || !record.window || !record.renderer ||
      state_->core.should_quit_ || state_->core.shutting_down_) return {};

  FrameStatistics frame_statistics;
  frame_statistics.render_pass_count = 1;

  ViewContext render_context = context_for_record(record);
  RenderTreeKind tree_kind = RenderTreeKind::none;
  std::optional<ElementId> rendered_root_id;
  std::size_t static_node_count = 0;
  std::optional<AccessibilityTreeSnapshot> accessibility_snapshot;
  if (view.supports_static_render()) {
    const StaticElementTreeView rendered = view.render_static(render_context);
    if (!record.active) return {};
    if (!rendered.empty() && rendered.valid()) {
      rendered_root_id = rendered.root_id();
      static_node_count = rendered.size();
      tree_kind = RenderTreeKind::static_element_tree;
      accessibility_snapshot = static_accessibility_snapshot_for(
          rendered,
          AccessibilitySnapshotOptions{
              .focused_element_id =
                  record.input.keyboard_focus_element_owner,
          });
    }
  }
  if (tree_kind != RenderTreeKind::static_element_tree) {
    AnyElement rendered = view.render(render_context);
    if (!record.active) return {};
    install_rendered_element_for_record(
        record, std::move(rendered), tree_kind, rendered_root_id);
  }

  state_->diagnostics.last_render_record_ = RenderRecord{
      .sequence = ++state_->diagnostics.render_sequence_,
      .view_id = record.root_view_id,
      .viewport_size = record.viewport_size,
      .tree_kind = tree_kind,
      .root_element_id = rendered_root_id,
      .static_node_count = static_node_count,
  };
  if (state_->core.after_render_callback_) {
    state_->core.after_render_callback_(context_for_record(record), *state_->diagnostics.last_render_record_);
  }
  if (!record.active) return {};
  if (accessibility_snapshot.has_value()) {
    update_platform_accessibility_tree_for_record(
        record, *accessibility_snapshot);
  } else {
    publish_element_tree_for_record(record);
  }
  if (!record.active) return {};

  std::vector<WindowControlRegion> window_control_regions;
  if (state_->core.should_quit_ || state_->core.shutting_down_) return {};
  if (state_->windows.surface_resizes_.has_pending(record.runtime_id))
    return std::unexpected(Error{ErrorCode::renderer_recovery_pending,
        "a renderer resize became pending during view callbacks"});
  auto result = render_view(
      *record.renderer,
      view,
      record.viewport_size,
      record.scale,
      nullptr,
      runtime_window_clear_color(record),
      &window_control_regions,
      element_tree_for_record(record),
      &frame_statistics);
  if (!result) {
    return std::unexpected(result.error());
  }
  if (!record.active) return {};
  if (!state_->windows.surface_resizes_.has_pending(record.runtime_id))
    state_->scheduling.frame_readiness_.clear(*this, record.runtime_id);
  // Lifetime scope retains the record, not its renderer/window after reentrant close.
  if (!record.active || record.window == nullptr || record.renderer == nullptr ||
      state_->core.should_quit_ || state_->core.shutting_down_) return {};
  runtime_publish_window_control_regions(
      record.window, std::move(window_control_regions));
  state_->diagnostics.last_renderer_frame_diagnostics_ =
      runtime_renderer_frame_diagnostic_snapshot(*record.renderer);

  state_->diagnostics.frame_index_ += 1;
  frame_statistics.frame_index = state_->diagnostics.frame_index_;
  if (state_->diagnostics.last_render_record_.has_value()) {
    state_->diagnostics.last_render_record_->statistics = frame_statistics;
  }
  state_->diagnostics.last_frame_statistics_ = frame_statistics;
  if (state_->core.after_frame_callback_) {
    state_->core.after_frame_callback_(context_for_record(record));
  }
  return {};
}

} // namespace cgpui
