#include "runtime_state_tasks_internal.hpp"
#include "runtime_async_io_internal.hpp"
#include "runtime_cross_thread_entity_internal.hpp"
#include "runtime_task_pool_internal.hpp"
#include "runtime_task_group_internal.hpp"

namespace cgpui {

WindowRuntime::RuntimeTaskState::RuntimeTaskState(WindowRuntime& owner)
    : cross_thread_entity_queue_(std::make_shared<detail::CrossThreadEntityQueueState>(owner)),
      async_io_registry_(std::make_unique<RuntimeAsyncIoRegistry>()),
      task_pool_(std::make_unique<RuntimeTaskPool>()),
      task_group_store_(std::make_unique<RuntimeTaskGroupStore>()) {}

WindowRuntime::RuntimeTaskState::~RuntimeTaskState() = default;

} // namespace cgpui
