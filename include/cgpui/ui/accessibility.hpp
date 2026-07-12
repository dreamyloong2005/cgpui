#pragma once

#include <optional>

namespace cgpui {

struct AccessibilityRangeValue {
  double value = 0.0;
  double minimum = 0.0;
  double maximum = 0.0;
  double small_change = 0.0;
  double large_change = 0.0;
};

struct AccessibilityPatternState {
  bool invokable = false;
  bool value_settable = false;
  std::optional<bool> toggled;
  std::optional<AccessibilityRangeValue> range;
};

} // namespace cgpui
