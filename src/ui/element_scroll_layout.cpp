#include "cgpui/ui/element_scroll_nodes.hpp"

#include <string>

namespace cgpui {
namespace {

constexpr std::size_t kUniformListRecyclingOverscanItems = 1;

[[nodiscard]] ElementKey key_for_child(const Element& child, std::size_t index) {
  if (child.key().has_value()) {
    return *child.key();
  }
  return ElementKey{.value = std::to_string(index)};
}

} // namespace

const UniformListLayoutSnapshot& ScrollableListElement::layout_snapshot()
    const {
  return layout_snapshot_;
}

const UniformListItemMeasurementCache& ScrollableListElement::measurement_cache()
    const {
  return measurement_cache_;
}

const UniformListSelectionState& ScrollableListElement::selection() const {
  return selection_;
}

LayoutOutput ScrollableListElement::layout(LayoutInput input) const {
  std::optional<UniformListScrollAnchor> scroll_anchor;
  Point offset;
  if (state_ != nullptr) {
    offset = state_->offset();
    scroll_anchor = capture_uniform_list_scroll_anchor(
        layout_snapshot_.items,
        offset);
  }

  LayoutInput content_input = input;
  content_input.constraints = {};
  const LayoutOutput content_output = content_.layout(content_input);
  Size preferred = style_.preferred_size;
  if (preferred.width == 0.0F) {
    preferred.width = content_output.size.width;
  }
  if (preferred.height == 0.0F) {
    preferred.height = content_output.size.height;
  }

  const LayoutOutput output{
      .size = constrain_size(preferred, input.constraints),
  };
  set_layout_bounds(Rect{
      .origin = output.origin,
      .size = output.size,
  });

  if (state_ != nullptr) {
    state_->set_viewport_size(output.size);
    state_->set_content_size(content_output.size);
    offset = state_->offset();
  }

  layout_snapshot_.items.clear();
  const auto children = content_.children();
  layout_snapshot_.items.reserve(children.size());
  for (std::size_t index = 0; index < children.size(); ++index) {
    const auto& child = children[index];
    if (!child) {
      continue;
    }
    const std::optional<Rect> bounds = child->layout_bounds();
    if (!bounds.has_value()) {
      continue;
    }
    layout_snapshot_.items.push_back(UniformListItemIdentity{
        .index = index,
        .key = key_for_child(*child, index),
        .element_id = child->id(),
        .content_bounds = *bounds,
    });
  }
  layout_snapshot_.measurements =
      measure_uniform_list_items(measurement_cache_, layout_snapshot_.items);
  if (state_ != nullptr && scroll_anchor.has_value()) {
    state_->set_offset(apply_uniform_list_scroll_anchor(
        layout_snapshot_.items,
        *scroll_anchor,
        offset));
    offset = state_->offset();
  }
  layout_snapshot_.visible_range = calculate_uniform_list_visible_range(
      layout_snapshot_.items,
      offset,
      output.size);
  layout_snapshot_.recycling_window = calculate_uniform_list_recycling_window(
      layout_snapshot_.visible_range,
      layout_snapshot_.measurements,
      kUniformListRecyclingOverscanItems);
  for (UniformListItemIdentity& item : layout_snapshot_.items) {
    item.visible = layout_snapshot_.visible_range.contains(item.index);
    item.recycled = layout_snapshot_.recycling_window.recycles(item.index);
    item.selected = selection_.selected(item.key);
  }

  content_.set_layout_bounds(Rect{
      .origin = {.x = -offset.x, .y = -offset.y},
      .size = content_output.size,
  });
  for (const auto& child : children) {
    if (const std::optional<Rect> bounds = child->layout_bounds();
        bounds.has_value()) {
      child->set_layout_bounds(Rect{
          .origin = {.x = bounds->origin.x - offset.x,
                     .y = bounds->origin.y - offset.y},
          .size = bounds->size,
      });
    }
  }

  return output;
}

} // namespace cgpui
