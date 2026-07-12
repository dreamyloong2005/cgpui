#pragma once

#include "cgpui/ui/runtime_ids.hpp"

#include <cstdint>

namespace cgpui {

struct AnimationCancellationDiagnostic {
  AnimationId id;
  std::uint64_t elapsed_ms = 0;
  std::uint64_t duration_ms = 0;
  bool timer_was_active = false;
};

} // namespace cgpui
