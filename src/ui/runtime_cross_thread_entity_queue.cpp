#include "ui_internal.hpp"
#include "runtime_cross_thread_entity_internal.hpp"

namespace cgpui::detail {

bool CrossThreadEntityQueueState::enqueue(
    CrossThreadEntityOperation operation) {
  if (!operation) return false;
  std::lock_guard lock(mutex_);
  if (runtime_ == nullptr) return false;
  operations_.push_back(std::move(operation));
  runtime_->request_platform_wakeup();
  return true;
}

std::vector<CrossThreadEntityOperation>
CrossThreadEntityQueueState::take_operations() {
  std::lock_guard lock(mutex_);
  std::vector<CrossThreadEntityOperation> operations;
  operations.swap(operations_);
  return operations;
}

void CrossThreadEntityQueueState::shutdown() {
  std::lock_guard lock(mutex_);
  runtime_ = nullptr;
  operations_.clear();
}

bool enqueue_cross_thread_entity_operation(
    const std::weak_ptr<CrossThreadEntityQueueState>& queue,
    CrossThreadEntityOperation operation) {
  const std::shared_ptr<CrossThreadEntityQueueState> state = queue.lock();
  return state != nullptr && state->enqueue(std::move(operation));
}

} // namespace cgpui::detail
