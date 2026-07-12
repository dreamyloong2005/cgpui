#include "cgpui/ui/async_context.hpp"

#include "cgpui/ui/runtime_context.hpp"

namespace cgpui {

TaskGroup AsyncContextCapability::create_task_group() const {
  return context_->create_task_group();
}

} // namespace cgpui
