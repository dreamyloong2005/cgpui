#pragma once

#include "cgpui/ui/runtime_ids.hpp"

#include <vector>

namespace cgpui::detail {

struct RuntimeTaskGroupRecord {
  TaskGroupId id;
  TaskGroupId parent_id{};
  std::vector<TaskGroupId> child_ids;
  std::vector<TaskId> task_ids;
  bool cancelled = false;
};

} // namespace cgpui::detail
