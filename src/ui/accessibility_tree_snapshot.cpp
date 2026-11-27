#include "cgpui/ui/element_core.hpp"

#include <algorithm>

namespace cgpui {

const AccessibilityNode* AccessibilityTreeSnapshot::node(ElementId element_id) const {
  const auto it = std::find_if(
      nodes.begin(),
      nodes.end(),
      [element_id](const AccessibilityNode& candidate) {
        return candidate.element_id == element_id;
      });
  return it == nodes.end() ? nullptr : &*it;
}

} // namespace cgpui
