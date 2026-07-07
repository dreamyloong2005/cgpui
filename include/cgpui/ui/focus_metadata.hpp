#pragma once

#include <optional>

namespace cgpui {

enum class FocusRingVisibility {
  automatic,
  visible,
  hidden,
};

struct FocusMetadata {
  std::optional<int> tab_index;
  FocusRingVisibility focus_ring = FocusRingVisibility::automatic;
};

} // namespace cgpui
