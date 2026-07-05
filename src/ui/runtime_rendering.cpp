#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::handle_resize(const WindowResized& event) {
  (void)resize_surface(event.size, event.scale);
}

void WindowRuntime::handle_redraw() {
  if (renderer_ == nullptr || should_quit_) {
    return;
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
        .constraints =
            {
                .max_size = viewport_size_,
            },
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
    after_frame_callback_(context());
  }
}

Result<void> WindowRuntime::resize_surface(Size size, DpiScale scale) {
  auto result = try_resize_surface(size, scale);
  if (!result) {
    fail_and_quit(result.error());
  }
  return result;
}

} // namespace cgpui
