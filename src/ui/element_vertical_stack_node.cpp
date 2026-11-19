#include "cgpui/ui/element_layout_nodes.hpp"

#include "element_layer_ordering.hpp"
#include "element_stack_box_internal.hpp"
#include "text_style_inheritance.hpp"

#include <algorithm>
#include <optional>
#include <utility>

namespace cgpui {

float VerticalStackElement::gap() const {
  return gap_;
}

void VerticalStackElement::set_gap(float gap) {
  gap_ = gap;
}

const Style& VerticalStackElement::box_style() const {
  return box_style_;
}

void VerticalStackElement::set_box_style(Style style) {
  box_style_ = std::move(style);
}

void VerticalStackElement::append_child(std::unique_ptr<Element> child) {
  if (child) {
    children_.push_back(std::move(child));
  }
}

std::span<const std::unique_ptr<Element>> VerticalStackElement::children()
    const {
  return children_;
}

void VerticalStackElement::inherit_text_style(const Style& style) {
  inherited_text_style_ = inherited_text_style(style);
}

LayoutOutput VerticalStackElement::layout(LayoutInput input) const {
  const Style child_text_style =
      merge_inherited_text_style(inherited_text_style_, box_style_);
  const Point content_offset =
      resolve_stack_box({}, box_style_, {}).content_offset;
  Size content_size;
  std::size_t relative_child_index = 0;
  for (const auto& child : children_) {
    child->inherit_text_style(child_text_style);
    LayoutInput child_input = input;
    child_input.constraints = {};
    const LayoutOutput child_output = child->layout(child_input);
    if (child->position() == Position::absolute) {
      continue;
    }
    if (relative_child_index > 0) {
      content_size.height += gap_;
    }
    child->place_layout(Rect{
        .origin = {.x = content_offset.x,
                   .y = content_offset.y + content_size.height},
        .size = child_output.size,
    });
    content_size.width = std::max(content_size.width, child_output.size.width);
    content_size.height += child_output.size.height;
    relative_child_index += 1;
  }

  const LayoutOutput output{
      .size = resolve_stack_box(content_size, box_style_, input.constraints)
                  .outer,
  };
  set_layout_bounds(Rect{
      .origin = output.origin,
      .size = output.size,
  });
  for (const auto& child : children_) {
    if (child->position() != Position::absolute) {
      continue;
    }
    child->inherit_text_style(child_text_style);
    LayoutInput child_input = input;
    child_input.constraints = {};
    const LayoutOutput child_output = child->layout(child_input);
    child->place_layout(Rect{
        .origin = absolute_origin(output.size, child->inset()),
        .size = child_output.size,
    });
  }
  return output;
}

ElementId VerticalStackElement::hit_test(Point point) const {
  const std::optional<Rect> bounds = layout_bounds();
  if (!bounds.has_value() || !contains(*bounds, point)) {
    return {};
  }

  for (const Element* child : hit_test_ordered_children(children_)) {
    const ElementId hit = child->hit_test(point);
    if (hit.value != 0) {
      return hit;
    }
  }
  return id();
}

} // namespace cgpui
