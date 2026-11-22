#include "ui_internal.hpp"

namespace cgpui {

TaskGroup WindowRuntimeContext::create_task_group() const {
  TaskGroup group = runtime.create_task_group();
  group.window_runtime_id_ = window_runtime_id;
  return group;
}

} // namespace cgpui
