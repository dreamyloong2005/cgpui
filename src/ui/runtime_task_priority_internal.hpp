#pragma once

#include "cgpui/ui/task_priority.hpp"

#include <array>
#include <cstddef>

namespace cgpui {

inline constexpr std::array runtime_task_priorities_descending{
    TaskPriority::high,
    TaskPriority::normal,
    TaskPriority::low,
};

constexpr std::size_t runtime_task_priority_index(TaskPriority priority) {
  switch (priority) {
    case TaskPriority::low: return 0;
    case TaskPriority::normal: return 1;
    case TaskPriority::high: return 2;
  }
  return 1;
}

} // namespace cgpui
