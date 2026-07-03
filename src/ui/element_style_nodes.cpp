#include "cgpui/ui/element_style_nodes.hpp"

#include <optional>
#include <utility>

namespace cgpui {

StyledElement::StyledElement(Style style, std::unique_ptr<Element> child)
    : style_state_(StyleState{.base = std::move(style)}),
      child_(std::move(child)) {}

StyledElement::StyledElement(
    StyleState style_state,
    std::unique_ptr<Element> child)
    : style_state_(std::move(style_state)),
      child_(std::move(child)) {}

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

Element* StyledElement::child() {
  return child_.get();
}

const Element* StyledElement::child() const {
  return child_.get();
}

LayoutOutput StyledElement::layout(LayoutInput input) const {
  const Style& base_style = style();
  Size content_size = base_style.preferred_size;
  if (child_) {
    const LayoutOutput child_output = child_->layout(input);
    content_size = child_output.size;
    child_->set_layout_bounds(Rect{
        .origin =
            {
                .x = base_style.margin.left + base_style.padding.left,
                .y = base_style.margin.top + base_style.padding.top,
            },
        .size = child_output.size,
    });
  }
  const Size preferred{
      .width = content_size.width + base_style.padding.left +
               base_style.padding.right + base_style.margin.left +
               base_style.margin.right,
      .height = content_size.height + base_style.padding.top +
                base_style.padding.bottom + base_style.margin.top +
                base_style.margin.bottom,
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

  const ElementId child_hit = child_ ? child_->hit_test(point) : ElementId{};
  return child_hit.value != 0 ? child_hit : Element::hit_test(point);
}

EventResult StyledElement::handle_event(
    const PlatformEvent& event,
    const ElementEventContext& context) {
  return child_ == nullptr || !child_->enabled()
             ? EventResult::unhandled()
             : child_->handle_event(event, context);
}

int StyledElement::z_index() const {
  return style().z_index;
}

int StyledElement::layer() const {
  return style().layer;
}

} // namespace cgpui
