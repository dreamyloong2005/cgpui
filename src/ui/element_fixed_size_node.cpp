#include "cgpui/ui/element_layout_nodes.hpp"

namespace cgpui {

FixedSizeElement::FixedSizeElement(Size preferred_size)
    : preferred_size_(preferred_size) {}

Size FixedSizeElement::preferred_size() const {
  return preferred_size_;
}

LayoutOutput FixedSizeElement::layout(LayoutInput input) const {
  const LayoutOutput output{
      .size = constrain_size(preferred_size_, input.constraints),
  };
  set_layout_bounds(Rect{
      .origin = output.origin,
      .size = output.size,
  });
  return output;
}

} // namespace cgpui
