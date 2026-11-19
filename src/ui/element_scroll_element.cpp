#include "cgpui/ui/element_scroll_nodes.hpp"

#include "cgpui/ui/paint.hpp"

namespace cgpui {

LayoutOutput ScrollElement::layout(LayoutInput input) const {
  if (child_ == nullptr) {
    const LayoutOutput output = Element::layout(input);
    if (state_ != nullptr) {
      state_->set_viewport_size(output.size);
      state_->set_content_size(output.size);
    }
    return output;
  }

  LayoutInput child_input = input;
  child_input.constraints = {};
  const LayoutOutput child_output = child_->layout(child_input);
  const LayoutOutput output{
      .size = constrain_size(child_output.size, input.constraints),
  };
  set_layout_bounds(Rect{
      .origin = output.origin,
      .size = output.size,
  });
  Point offset;
  if (state_ != nullptr) {
    state_->set_viewport_size(output.size);
    state_->set_content_size(child_output.size);
    // Sizes may have shrunk since the offset was set.
    state_->set_offset(state_->offset());
    offset = state_->offset();
  }
  // place_layout moves the child's whole subtree (window coordinates).
  child_->place_layout(Rect{
      .origin = {.x = child_output.origin.x - offset.x,
                 .y = child_output.origin.y - offset.y},
      .size = child_output.size,
  });
  return output;
}

void ScrollElement::paint(PaintList& paint_list) const {
  if (!child_) {
    return;
  }
  const std::optional<Rect> bounds = layout_bounds();
  if (bounds.has_value()) {
    paint_list.push_clip(*bounds);
  }
  child_->paint(paint_list);
  if (bounds.has_value()) {
    paint_list.pop_clip();
  }
}

} // namespace cgpui
