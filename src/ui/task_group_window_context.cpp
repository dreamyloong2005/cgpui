#include "ui_internal.hpp"

namespace cgpui {

TaskCompletionCallback TaskGroup::bind_callback(TaskCompletionCallback callback) const {
  auto* runtime = attached_runtime();
  if (runtime == nullptr) return {};
  const auto id = window_runtime_id_.value == 0
      ? runtime->root_window_runtime_id() : window_runtime_id_;
  return runtime->bind_window_callback(id, std::move(callback));
}

} // namespace cgpui
