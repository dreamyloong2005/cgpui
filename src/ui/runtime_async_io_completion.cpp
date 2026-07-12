#include "ui_internal.hpp"
#include "runtime_async_io_internal.hpp"
#include "runtime_task_priority_internal.hpp"

namespace cgpui {

void WindowRuntime::drain_async_io_completions() {
  std::vector<RuntimeAsyncIoRegistry::Completion> completions =
      async_io_registry_->take_completions();
  for (const TaskPriority priority : runtime_task_priorities_descending) {
    for (const auto& completion : completions) {
      if (completion.priority != priority) continue;
      AsyncIoCompletionCallback callback =
          async_io_registry_->complete(completion.id);
      if (callback) callback(context(), completion.result);
    }
  }
}

} // namespace cgpui
