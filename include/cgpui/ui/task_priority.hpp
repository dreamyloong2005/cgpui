#pragma once

#include <cstdint>

namespace cgpui {

enum class TaskPriority : std::uint8_t {
  low,
  normal,
  high,
};

} // namespace cgpui
