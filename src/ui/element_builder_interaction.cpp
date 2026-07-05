#include "cgpui/ui/element_builder_core.hpp"

#include <utility>

namespace cgpui {

ElementBuilder ElementBuilder::enabled(bool value) && {
  enabled_ = value;
  return std::move(*this);
}

ElementBuilder ElementBuilder::disabled() && {
  enabled_ = false;
  return std::move(*this);
}

ElementBuilder ElementBuilder::focusable() && {
  focusable_ = true;
  return std::move(*this);
}

ElementBuilder ElementBuilder::on_click(ClickHandler handler) && {
  click_handler_ = std::move(handler);
  return std::move(*this);
}

ElementBuilder ElementBuilder::on_pointer_down(
    PointerButtonHandler handler) && {
  pointer_down_handler_ = std::move(handler);
  return std::move(*this);
}

ElementBuilder ElementBuilder::on_pointer_up(PointerButtonHandler handler) && {
  pointer_up_handler_ = std::move(handler);
  return std::move(*this);
}

ElementBuilder ElementBuilder::on_pointer_move(PointerMoveHandler handler) && {
  pointer_move_handler_ = std::move(handler);
  return std::move(*this);
}

ElementBuilder ElementBuilder::on_key(KeyHandler handler) && {
  key_handler_ = std::move(handler);
  return std::move(*this);
}

ElementBuilder ElementBuilder::child(std::unique_ptr<Element> child) && {
  if (child) {
    children_.push_back(std::move(child));
  }
  return std::move(*this);
}

ElementBuilder ElementBuilder::child(ElementBuilder child) && {
  return std::move(*this).child(into_element(std::move(child)));
}

ElementBuilder ElementBuilder::children(std::vector<AnyElement> children) && {
  for (auto& child : children) {
    if (child) {
      children_.push_back(std::move(child));
    }
  }
  return std::move(*this);
}

} // namespace cgpui
