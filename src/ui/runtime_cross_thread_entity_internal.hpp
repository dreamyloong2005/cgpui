#pragma once

#include "cgpui/ui/cross_thread_entity.hpp"

#include <mutex>
#include <vector>

namespace cgpui::detail {

class CrossThreadEntityQueueState {
 public:
  explicit CrossThreadEntityQueueState(WindowRuntime& runtime)
      : runtime_(&runtime) {}

  [[nodiscard]] bool enqueue(CrossThreadEntityOperation operation);
  [[nodiscard]] std::vector<CrossThreadEntityOperation> take_operations();
  void shutdown();

 private:
  std::mutex mutex_;
  WindowRuntime* runtime_ = nullptr;
  std::vector<CrossThreadEntityOperation> operations_;
};

} // namespace cgpui::detail
