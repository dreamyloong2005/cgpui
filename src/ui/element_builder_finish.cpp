#include "cgpui/ui/element_builder_core.hpp"

#include <utility>

namespace cgpui {

ElementBuilder::ElementBuilder(Kind kind) : kind_(kind) {}

void ElementBuilder::apply_element_attributes(Element& element) const {
  element.set_enabled(enabled_);
  element.set_key(key_);
  element.set_flex_grow(style_state_.base.flex_grow);
  element.set_flex_shrink(style_state_.base.flex_shrink);
  element.set_position(style_state_.base.position);
  element.set_inset(style_state_.base.inset);
  element.set_z_index(style_state_.base.z_index);
  element.set_layer(style_state_.base.layer);
}

std::unique_ptr<Element> ElementBuilder::finish(
    std::unique_ptr<Element> element) const {
  apply_element_attributes(*element);
  if (click_handler_) {
    auto click_element =
        std::make_unique<ClickElement>(std::move(element), click_handler_);
    apply_element_attributes(*click_element);
    element = std::move(click_element);
  }
  if (pointer_down_handler_ || pointer_up_handler_ || pointer_move_handler_) {
    auto pointer_element = std::make_unique<PointerElement>(
        std::move(element),
        pointer_down_handler_,
        pointer_up_handler_,
        pointer_move_handler_);
    apply_element_attributes(*pointer_element);
    element = std::move(pointer_element);
  }
  if (key_handler_) {
    auto key_element =
        std::make_unique<KeyElement>(std::move(element), key_handler_);
    apply_element_attributes(*key_element);
    element = std::move(key_element);
  }
  if (focusable_) {
    auto focusable_element =
        std::make_unique<FocusableElement>(std::move(element));
    apply_element_attributes(*focusable_element);
    element = std::move(focusable_element);
  }
  return element;
}

} // namespace cgpui
