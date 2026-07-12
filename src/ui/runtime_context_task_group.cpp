#include "ui_internal.hpp"

namespace cgpui {

TaskGroup WindowRuntimeContext::create_task_group() const {
  return runtime.create_task_group();
}

} // namespace cgpui
