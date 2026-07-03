#include "cgpui/ui/element_layout_nodes.hpp"

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

void VerticalStackElement::append_child(std::unique_ptr<Element> child) {
  if (child) {
    children_.push_back(std::move(child));
  }
}

std::span<const std::unique_ptr<Element>> VerticalStackElement::children()
    const {
  return children_;
}

LayoutOutput VerticalStackElement::layout(LayoutInput input) const {
  Size content_size;
  std::size_t relative_child_index = 0;
  for (const auto& child : children_) {
    const LayoutOutput child_output =
        child->layout(LayoutInput{.scale = input.scale});
    if (child->position() == Position::absolute) {
      continue;
    }
    if (relative_child_index > 0) {
      content_size.height += gap_;
    }
    child->set_layout_bounds(Rect{
        .origin = {.x = 0.0F, .y = content_size.height},
        .size = child_output.size,
    });
    content_size.width = std::max(content_size.width, child_output.size.width);
    content_size.height += child_output.size.height;
    relative_child_index += 1;
  }

  const LayoutOutput output{
      .size = constrain_size(content_size, input.constraints),
  };
  set_layout_bounds(Rect{
      .origin = output.origin,
      .size = output.size,
  });
  for (const auto& child : children_) {
    if (child->position() != Position::absolute) {
      continue;
    }
    const LayoutOutput child_output =
        child->layout(LayoutInput{.scale = input.scale});
    child->set_layout_bounds(Rect{
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

  for (auto iterator = children_.rbegin(); iterator != children_.rend();
       ++iterator) {
    const ElementId hit = (*iterator)->hit_test(point);
    if (hit.value != 0) {
      return hit;
    }
  }
  return id();
}

} // namespace cgpui
