#include "cgpui/ui/element_builder_core.hpp"

#include <utility>

namespace cgpui {

std::unique_ptr<Element> ElementBuilder::build() && {
  if (kind_ == Kind::fixed_size) {
    return finish(std::make_unique<FixedSizeElement>(size_));
  }
  if (kind_ == Kind::text) {
    return finish(std::make_unique<TextElement>(text_model_, style_state_.base));
  }
  if (kind_ == Kind::child_view) {
    return finish(std::make_unique<ChildViewElement>(child_view_id_, size_));
  }
  if (kind_ == Kind::v_stack) {
    auto element = std::make_unique<VerticalStackElement>();
    element->set_gap(style_state_.base.gap);
    for (auto& child : children_) {
      element->append_child(std::move(child));
    }
    return finish(std::move(element));
  }
  if (kind_ == Kind::row || kind_ == Kind::column) {
    auto element = std::make_unique<FlexElement>(
        kind_ == Kind::row ? FlexDirection::row : FlexDirection::column);
    element->set_gap(style_state_.base.gap);
    element->set_align_items(style_state_.base.align_items);
    element->set_justify_content(style_state_.base.justify_content);
    for (auto& child : children_) {
      element->append_child(std::move(child));
    }
    return finish(std::move(element));
  }

  std::unique_ptr<Element> child;
  if (!children_.empty()) {
    child = std::move(children_.front());
  }
  auto element = std::make_unique<StyledElement>(style_state_, std::move(child));
  element->set_style_classes(style_classes_);
  element->set_inline_style(inline_style_);
  return finish(std::move(element));
}

} // namespace cgpui
