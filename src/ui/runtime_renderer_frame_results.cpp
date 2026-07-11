#include "ui_internal.hpp"

namespace cgpui {

Result<void> WindowRuntime::try_draw_frame() {
  if (renderer_ == nullptr || should_quit_) {
    return {};
  }
  begin_frame_scheduling();

  FrameStatistics frame_statistics;
  frame_statistics.render_pass_count = 1;

  ViewContext render_context = context();
  RenderTreeKind tree_kind = RenderTreeKind::none;
  std::optional<ElementId> rendered_root_id;
  std::size_t static_node_count = 0;

  const StaticRenderInstallResult static_render =
      install_static_render_tree(view_, render_context);
  if (static_render.installed) {
    rendered_root_id = static_render.root_id;
    static_node_count = static_render.node_count;
    tree_kind = RenderTreeKind::static_element_tree;
  } else {
    clear_static_element_tree();
    AnyElement rendered = view_.render(render_context);
    if (rendered != nullptr) {
      auto tree = std::make_unique<ElementTree>();
      rendered_root_id = tree->set_root(std::move(rendered));
      set_element_tree(std::move(tree));
      tree_kind = RenderTreeKind::dynamic_element_tree;
    }
  }

  last_render_record_ = RenderRecord{
      .sequence = ++render_sequence_,
      .view_id = root_view_id_,
      .viewport_size = viewport_size_,
      .tree_kind = tree_kind,
      .root_element_id = rendered_root_id,
      .static_node_count = static_node_count,
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
    abort_frame_scheduling();
    return std::unexpected(result.error());
  }
  last_renderer_frame_diagnostics_ =
      runtime_renderer_frame_diagnostic_snapshot(*renderer_);

  frame_index_ += 1;
  frame_statistics.frame_index = frame_index_;
  if (last_render_record_.has_value()) {
    last_render_record_->statistics = frame_statistics;
  }
  last_frame_statistics_ = frame_statistics;
  if (after_frame_callback_) {
    after_frame_callback_(context());
  }
  complete_frame_scheduling();
  return {};
}

Result<void> WindowRuntime::try_draw_frame_for_record(
    WindowRuntimeRecord& record,
    View& view) {
  if (!record.active || record.window == nullptr || record.renderer == nullptr ||
      should_quit_) {
    return {};
  }

  FrameStatistics frame_statistics;
  frame_statistics.render_pass_count = 1;

  ViewContext render_context = context_for_record(record);
  RenderTreeKind tree_kind = RenderTreeKind::none;
  std::optional<ElementId> rendered_root_id;
  std::size_t static_node_count = 0;
  if (view.supports_static_render()) {
    const StaticElementTreeView rendered = view.render_static(render_context);
    if (!rendered.empty() && rendered.valid()) {
      rendered_root_id = rendered.root_id();
      static_node_count = rendered.size();
      tree_kind = RenderTreeKind::static_element_tree;
    }
  }
  if (tree_kind != RenderTreeKind::static_element_tree) {
    (void)view.render(render_context);
  }

  last_render_record_ = RenderRecord{
      .sequence = ++render_sequence_,
      .view_id = record.root_view_id,
      .viewport_size = record.viewport_size,
      .tree_kind = tree_kind,
      .root_element_id = rendered_root_id,
      .static_node_count = static_node_count,
  };
  if (after_render_callback_) {
    after_render_callback_(context_for_record(record), *last_render_record_);
  }

  auto result = render_view(
      *record.renderer,
      view,
      record.viewport_size,
      record.scale,
      &frame_statistics);
  if (!result) {
    return std::unexpected(result.error());
  }
  last_renderer_frame_diagnostics_ =
      runtime_renderer_frame_diagnostic_snapshot(*record.renderer);

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
