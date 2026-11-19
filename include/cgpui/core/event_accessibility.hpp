#pragma once

#include <cstdint>
#include <string>

namespace cgpui {

enum class AccessibilityActionKind {
  invoke,
  toggle,
  set_value,
  set_range_value,
  // UIA ExpandCollapse / SelectionItem, AT-SPI Action "expand" / "collapse"
  // and Selection.
  expand,
  collapse,
  select,
  add_to_selection,
  remove_from_selection,
};

struct AccessibilityActionRequested {
  AccessibilityActionKind kind = AccessibilityActionKind::invoke;
  std::uint64_t element_id = 0;
  std::string value;
  double numeric_value = 0.0;
};

} // namespace cgpui
