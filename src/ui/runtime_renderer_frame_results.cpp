#include "ui_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

Result<void> WindowRuntime::try_draw_frame() {
  if (state_->windows.renderer_ == nullptr || state_->core.should_quit_) {
    return {};
  }
  const RuntimeWindowCallbackScope callback_scope(*this);
  begin_frame_scheduling();

  FrameStatistics frame_statistics;
  frame_statistics.render_pass_count = 1;

  ViewContext render_context = context();
  RenderTreeKind tree_kind = RenderTreeKind::none;
  std::optional<ElementId> rendered_root_id;
  std::size_t static_node_count = 0;

  const StaticRenderInstallResult static_render =
      install_static_render_tree(state_->core.view_, render_context);
  if (static_render.installed) {
    rendered_root_id = static_render.root_id;
    static_node_count = static_render.node_count;
    tree_kind = RenderTreeKind::static_element_tree;
  } else {
    clear_static_element_tree();
    const bool had_view_rendered_element_tree = state_->elements.view_rendered_element_tree_;
    AnyElement rendered = state_->core.view_.render(render_context);
    if (rendered != nullptr) {
      auto tree = std::make_unique<ElementTree>();
      rendered_root_id = tree->set_root(std::move(rendered));
      set_element_tree(std::move(tree));
      state_->elements.view_rendered_element_tree_ = true;
      tree_kind = RenderTreeKind::dynamic_element_tree;
    } else if (had_view_rendered_element_tree) {
      set_element_tree(nullptr);
    }
  }

  state_->diagnostics.last_render_record_ = RenderRecord{
      .sequence = ++state_->diagnostics.render_sequence_,
      .view_id = state_->views.root_view_id_,
      .viewport_size = state_->windows.viewport_size_,
      .tree_kind = tree_kind,
      .root_element_id = rendered_root_id,
      .static_node_count = static_node_count,
  };
  if (state_->core.after_render_callback_) {
    state_->core.after_render_callback_(context(), *state_->diagnostics.last_render_record_);
  }

  layout_element_tree_with_animations(frame_statistics);
  update_platform_accessibility_tree();
  apply_focused_text_ime_placement();

  std::vector<WindowControlRegion> window_control_regions;
  const auto* root_record = find_window_runtime_record(state_->windows.root_window_runtime_id_);
  auto result = render_view(
      *state_->windows.renderer_,
      state_->core.view_,
      state_->windows.viewport_size_,
      state_->windows.scale_,
      nullptr,
      root_record != nullptr ? runtime_window_clear_color(*root_record)
                             : runtime_window_clear_color(state_->windows.window_),
      &window_control_regions,
      element_tree(),
      &frame_statistics);
  if (!result) {
    abort_frame_scheduling();
    return std::unexpected(result.error());
  }
  state_->scheduling.frame_readiness_.clear(*this, state_->windows.root_window_runtime_id_);
  // Timer cancellation can synchronously reenter platform lifecycle callbacks.
  if (state_->core.should_quit_ || state_->core.shutting_down_ ||
      state_->windows.window_ == nullptr || state_->windows.renderer_ == nullptr) {
    abort_frame_scheduling();
    return {};
  }
  runtime_publish_window_control_regions(
      state_->windows.window_, std::move(window_control_regions));
  state_->diagnostics.last_renderer_frame_diagnostics_ =
      runtime_renderer_frame_diagnostic_snapshot(*state_->windows.renderer_);

  state_->diagnostics.frame_index_ += 1;
  frame_statistics.frame_index = state_->diagnostics.frame_index_;
  if (state_->diagnostics.last_render_record_.has_value()) {
    state_->diagnostics.last_render_record_->statistics = frame_statistics;
  }
  state_->diagnostics.last_frame_statistics_ = frame_statistics;
  if (state_->core.after_frame_callback_) {
    state_->core.after_frame_callback_(context());
  }
  complete_frame_scheduling();
  return {};
}

} // namespace cgpui
