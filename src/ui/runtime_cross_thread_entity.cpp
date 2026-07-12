#include "ui_internal.hpp"
#include "runtime_cross_thread_entity_internal.hpp"

namespace cgpui {

void WindowRuntime::drain_cross_thread_entity_operations() {
  while (!should_quit_) {
    std::vector<detail::CrossThreadEntityOperation> operations =
        cross_thread_entity_queue_->take_operations();
    if (operations.empty()) return;
    for (auto& operation : operations) {
      if (operation) operation(context());
    }
  }
}

} // namespace cgpui
