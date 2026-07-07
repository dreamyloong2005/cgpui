#include "cgpui/ui/element_item_nodes.hpp"

#include "text_style_inheritance.hpp"

namespace cgpui {

LayoutOutput ItemElement::layout(LayoutInput input) const {
  Size content_size = style_state_.base.preferred_size;
  if (child_) {
    child_->inherit_text_style(
        merge_inherited_text_style(inherited_text_style_, style_state_.base));
    const LayoutOutput child_output = child_->layout(input);
    content_size = child_output.size;
    child_->set_layout_bounds(Rect{
        .origin =
            {
                .x = style_state_.base.margin.left +
                     style_state_.base.padding.left,
                .y = style_state_.base.margin.top +
                     style_state_.base.padding.top,
            },
        .size = child_output.size,
    });
  }
  const Size preferred{
      .width = content_size.width + style_state_.base.padding.left +
               style_state_.base.padding.right +
               style_state_.base.margin.left + style_state_.base.margin.right,
      .height = content_size.height + style_state_.base.padding.top +
                style_state_.base.padding.bottom +
                style_state_.base.margin.top + style_state_.base.margin.bottom,
  };
  const LayoutOutput output{
      .size = constrain_size(preferred, input.constraints),
  };
  set_layout_bounds(Rect{
      .origin = output.origin,
      .size = output.size,
  });
  return output;
}

} // namespace cgpui
