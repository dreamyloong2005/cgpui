#include "ui_internal.hpp"
#include "runtime_task_group_internal.hpp"

namespace cgpui {
namespace detail {
struct TaskGroupLifetime {};
} // namespace detail

WindowRuntime::RuntimeTaskGroupStore::RuntimeTaskGroupStore()
    : lifetime_(std::make_shared<detail::TaskGroupLifetime>()) {}

std::weak_ptr<detail::TaskGroupLifetime>
WindowRuntime::RuntimeTaskGroupStore::lifetime_token() const {
  return lifetime_;
}

void WindowRuntime::RuntimeTaskGroupStore::detach_handles() {
  lifetime_.reset();
}

TaskGroup::TaskGroup(WindowRuntime& runtime, TaskGroupId id)
    : runtime_(&runtime), runtime_lifetime_(runtime.state_->tasks.task_group_store_->lifetime_token()),
      id_(id) {}

WindowRuntime* TaskGroup::attached_runtime() const {
  // TaskGroup is a UI-thread facade. This prevents late RAII destruction from
  // touching a dead runtime; it does not synchronize concurrent runtime deletion.
  return runtime_lifetime_.expired() ? nullptr : runtime_;
}

} // namespace cgpui
