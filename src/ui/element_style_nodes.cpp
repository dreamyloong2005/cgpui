#include "cgpui/ui/element_style_nodes.hpp"

#include "element_layer_ordering.hpp"
#include "text_style_inheritance.hpp"

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>

namespace cgpui {

namespace {

[[nodiscard]] LayoutConstraints merge_style_constraints(
    LayoutConstraints constraints,
    const Style& style) {
  constraints.min_size.width =
      std::max(constraints.min_size.width, style.min_size.width);
  constraints.min_size.height =
      std::max(constraints.min_size.height, style.min_size.height);
  constraints.max_size.width =
      std::min(constraints.max_size.width, style.max_size.width);
  constraints.max_size.height =
      std::min(constraints.max_size.height, style.max_size.height);
  constraints.max_size.width =
      std::max(constraints.max_size.width, constraints.min_size.width);
  constraints.max_size.height =
      std::max(constraints.max_size.height, constraints.min_size.height);
  return constraints;
}

[[nodiscard]] float resolve_percentage_axis(
    std::optional<float> percent,
    float fallback,
    float parent_max) {
  if (!percent.has_value() || !std::isfinite(parent_max)) {
    return fallback;
  }
  return std::max(0.0F, parent_max * (*percent / 100.0F));
}

[[nodiscard]] Size resolve_percentage_size(
    Size fallback,
    const PercentageSize& percentage_size,
    LayoutConstraints constraints) {
  return Size{
      .width = resolve_percentage_axis(
          percentage_size.width,
          fallback.width,
          constraints.max_size.width),
      .height = resolve_percentage_axis(
          percentage_size.height,
          fallback.height,
          constraints.max_size.height),
  };
}

[[nodiscard]] bool is_positioned_out_of_flow(Position position) {
  return position == Position::absolute || position == Position::fixed;
}

} // namespace

StyledElement::StyledElement(Style style, std::unique_ptr<Element> child)
    : style_state_(StyleState{.base = std::move(style)}) {
  if (child != nullptr) {
    children_.push_back(std::move(child));
  }
}

StyledElement::StyledElement(
    Style style,
    std::vector<std::unique_ptr<Element>> children)
    : style_state_(StyleState{.base = std::move(style)}),
      children_(std::move(children)) {}

StyledElement::StyledElement(
    StyleState style_state,
    std::unique_ptr<Element> child)
    : style_state_(std::move(style_state)) {
  if (child != nullptr) {
    children_.push_back(std::move(child));
  }
}

StyledElement::StyledElement(
    StyleState style_state,
    std::vector<std::unique_ptr<Element>> children)
    : style_state_(std::move(style_state)),
      children_(std::move(children)) {}

const Style& StyledElement::style() const {
  return style_state_.base;
}

const StyleState& StyledElement::style_state() const {
  return style_state_;
}

const StyleClasses& StyledElement::style_classes() const {
  return style_classes_;
}

const StyleOverlay& StyledElement::inline_style() const {
  return inline_style_;
}

void StyledElement::set_style_classes(StyleClasses classes) {
  style_classes_ = std::move(classes);
}

void StyledElement::set_inline_style(StyleOverlay style) {
  inline_style_ = std::move(style);
}

Style StyledElement::resolved_style(
    const StyleCascade& cascade,
    StyleStateFlags flags) const {
  return cgpui::resolved_style(
      cascade,
      style_state_,
      style_classes_,
      inline_style_,
      flags);
}

Style StyledElement::resolved_style(
    const StyleCascade& cascade,
    StyleStateFlags flags,
    const Theme& theme) const {
  return cgpui::resolved_style(
      cascade,
      style_state_,
      style_classes_,
      inline_style_,
      flags,
      theme);
}

Element* StyledElement::child() {
  return children_.empty() ? nullptr : children_.front().get();
}

const Element* StyledElement::child() const {
  return children_.empty() ? nullptr : children_.front().get();
}

std::vector<std::unique_ptr<Element>>& StyledElement::children() {
  return children_;
}

const std::vector<std::unique_ptr<Element>>& StyledElement::children() const {
  return children_;
}

LayoutOutput StyledElement::layout(LayoutInput input) const {
  const Style& base_style = style();
  const Style child_text_style =
      merge_inherited_text_style(inherited_text_style_, base_style);
  Size content_size = base_style.preferred_size;
  if (!children_.empty()) {
    float child_y = 0.0F;
    float content_width = 0.0F;
    float content_height = 0.0F;
    std::size_t flow_child_count = 0;
    for (const auto& child_ptr : children_) {
      Element& child = *child_ptr;
      child.inherit_text_style(child_text_style);
      if (is_positioned_out_of_flow(child.position())) {
        continue;
      }
      if (flow_child_count > 0) {
        child_y += base_style.gap;
      }
      const LayoutOutput child_output = child.layout(input);
      child.set_layout_bounds(Rect{
          .origin =
              {
                  .x = base_style.margin.left + base_style.padding.left,
                  .y = base_style.margin.top + base_style.padding.top + child_y,
              },
          .size = child_output.size,
      });
      content_width = std::max(content_width, child_output.size.width);
      content_height = child_y + child_output.size.height;
      child_y = content_height;
      flow_child_count += 1;
    }
    if (flow_child_count > 0) {
      content_size = Size{
          .width = content_width,
          .height = content_height,
      };
    }
  }
  content_size = resolve_percentage_size(
      content_size,
      base_style.percentage_size,
      input.constraints);
  const Size preferred{
      .width = content_size.width + base_style.padding.left +
               base_style.padding.right + base_style.margin.left +
               base_style.margin.right,
      .height = content_size.height + base_style.padding.top +
                base_style.padding.bottom + base_style.margin.top +
                base_style.margin.bottom,
  };
  const LayoutOutput output{
      .size = constrain_size(
          preferred,
          merge_style_constraints(input.constraints, base_style)),
  };
  set_layout_bounds(Rect{
      .origin = output.origin,
      .size = output.size,
  });
  for (const auto& child_ptr : children_) {
    Element& child = *child_ptr;
    if (!is_positioned_out_of_flow(child.position())) {
      continue;
    }
    child.inherit_text_style(child_text_style);
    const LayoutOutput child_output = child.layout(input);
    child.set_layout_bounds(Rect{
        .origin = absolute_origin(output.size, child.inset()),
        .size = child_output.size,
    });
  }
  return output;
}

void StyledElement::inherit_text_style(const Style& style) {
  inherited_text_style_ = inherited_text_style(style);
}

ElementId StyledElement::hit_test(Point point) const {
  if (style().overflow == Overflow::hidden) {
    const std::optional<Rect> bounds = layout_bounds();
    if (!bounds.has_value()) {
      return {};
    }
    const Rect clip_bounds =
        style().clip_rect.has_value() ? *style().clip_rect : *bounds;
    if (!contains(clip_bounds, point)) {
      return {};
    }
  }

  for (const Element* child : hit_test_ordered_children(children_)) {
    const ElementId child_hit = child->hit_test(point);
    if (child_hit.value != 0) {
      return child_hit;
    }
  }
  return Element::hit_test(point);
}

EventResult StyledElement::handle_event(
    const PlatformEvent& event,
    const ElementEventContext& context) {
  for (Element* child : event_ordered_children(children_)) {
    if (!child->enabled()) {
      continue;
    }
    const EventResult result = child->handle_event(event, context);
    if (result.consumed || result.cancelled) {
      return result;
    }
  }
  return EventResult::unhandled();
}

int StyledElement::z_index() const {
  return style().z_index;
}

int StyledElement::layer() const {
  return style().layer;
}

} // namespace cgpui
