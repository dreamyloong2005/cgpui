#include "cgpui/ui/element_layout_nodes.hpp"

#include <algorithm>

namespace cgpui {

LayoutOutput FlexElement::layout(LayoutInput input) const {
  std::vector<Size> child_sizes;
  child_sizes.reserve(children_.size());
  Size content_size;
  float total_flex_grow = 0.0F;
  float total_flex_shrink = 0.0F;
  std::vector<std::size_t> relative_indices;
  relative_indices.reserve(children_.size());
  std::size_t relative_child_index = 0;
  for (std::size_t child_index = 0; child_index < children_.size();
       ++child_index) {
    const auto& child = children_[child_index];
    const LayoutOutput child_output =
        child->layout(LayoutInput{.scale = input.scale});
    child_sizes.push_back(child_output.size);
    if (child->position() == Position::absolute) {
      continue;
    }
    relative_indices.push_back(child_index);
    if (relative_child_index > 0) {
      if (direction_ == FlexDirection::row) {
        content_size.width += gap_;
      } else {
        content_size.height += gap_;
      }
    }
    total_flex_grow += std::max(0.0F, child->flex_grow());
    total_flex_shrink += std::max(0.0F, child->flex_shrink());

    if (direction_ == FlexDirection::row) {
      content_size.width += child_output.size.width;
      content_size.height =
          std::max(content_size.height, child_output.size.height);
    } else {
      content_size.width =
          std::max(content_size.width, child_output.size.width);
      content_size.height += child_output.size.height;
    }
    relative_child_index += 1;
  }

  const LayoutOutput output{
      .size = constrain_size(content_size, input.constraints),
  };
  set_layout_bounds(Rect{
      .origin = output.origin,
      .size = output.size,
  });

  const bool row = direction_ == FlexDirection::row;
  const float output_main = row ? output.size.width : output.size.height;
  const float output_cross = row ? output.size.height : output.size.width;
  const float content_main = row ? content_size.width : content_size.height;
  const float positive_free_space = std::max(0.0F, output_main - content_main);
  const float overflow_space = std::max(0.0F, content_main - output_main);
  for (std::size_t index = 0; index < child_sizes.size(); ++index) {
    if (children_[index]->position() == Position::absolute) {
      continue;
    }
    Size& child_size = child_sizes[index];
    float child_main = row ? child_size.width : child_size.height;
    if (positive_free_space > 0.0F && total_flex_grow > 0.0F) {
      child_main +=
          positive_free_space *
          (std::max(0.0F, children_[index]->flex_grow()) / total_flex_grow);
    } else if (overflow_space > 0.0F && total_flex_shrink > 0.0F) {
      child_main -=
          overflow_space *
          (std::max(0.0F, children_[index]->flex_shrink()) /
           total_flex_shrink);
      child_main = std::max(0.0F, child_main);
    }
    if (row) {
      child_size.width = child_main;
    } else {
      child_size.height = child_main;
    }
  }

  float laid_out_main = 0.0F;
  for (std::size_t relative_index = 0; relative_index < relative_indices.size();
       ++relative_index) {
    const std::size_t index = relative_indices[relative_index];
    laid_out_main += row ? child_sizes[index].width : child_sizes[index].height;
    if (relative_index + 1 < relative_indices.size()) {
      laid_out_main += gap_;
    }
  }
  const float free_space = std::max(0.0F, output_main - laid_out_main);
  float main_offset = 0.0F;
  float extra_gap = 0.0F;
  if (justify_content_ == JustifyContent::center) {
    main_offset = free_space / 2.0F;
  } else if (justify_content_ == JustifyContent::end) {
    main_offset = free_space;
  } else if (justify_content_ == JustifyContent::space_between &&
             relative_indices.size() > 1) {
    extra_gap = free_space / static_cast<float>(relative_indices.size() - 1);
  }

  float cursor = main_offset;
  for (std::size_t relative_index = 0; relative_index < relative_indices.size();
       ++relative_index) {
    const std::size_t index = relative_indices[relative_index];
    const Size child_size = child_sizes[index];
    const float child_cross = row ? child_size.height : child_size.width;
    float cross_offset = 0.0F;
    if (align_items_ == AlignItems::center) {
      cross_offset = std::max(0.0F, output_cross - child_cross) / 2.0F;
    } else if (align_items_ == AlignItems::end) {
      cross_offset = std::max(0.0F, output_cross - child_cross);
    }

    children_[index]->set_layout_bounds(Rect{
        .origin =
            row ? Point{.x = cursor, .y = cross_offset}
                : Point{.x = cross_offset, .y = cursor},
        .size = child_size,
    });

    cursor += row ? child_size.width : child_size.height;
    if (relative_index + 1 < relative_indices.size()) {
      cursor += gap_ + extra_gap;
    }
  }
  for (std::size_t index = 0; index < children_.size(); ++index) {
    if (children_[index]->position() != Position::absolute) {
      continue;
    }
    children_[index]->set_layout_bounds(Rect{
        .origin = absolute_origin(output.size, children_[index]->inset()),
        .size = child_sizes[index],
    });
  }
  return output;
}

} // namespace cgpui
