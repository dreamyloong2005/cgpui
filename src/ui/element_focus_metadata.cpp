#include "cgpui/ui/element_core.hpp"

#include <utility>

namespace cgpui {

FocusMetadata Element::focus_metadata() const {
  return focus_metadata_;
}

void Element::set_focus_metadata(FocusMetadata metadata) {
  focus_metadata_ = std::move(metadata);
}

} // namespace cgpui
