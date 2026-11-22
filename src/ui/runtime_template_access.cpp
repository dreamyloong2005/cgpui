#include "ui_internal.hpp"

namespace cgpui {

ElementTree* WindowRuntime::mutable_element_tree() {
  return state_->elements.owned_element_tree_.get();
}

std::shared_ptr<detail::CrossThreadEntityQueueState> WindowRuntime::cross_thread_entity_queue() const {
  return state_->tasks.cross_thread_entity_queue_;
}

} // namespace cgpui
