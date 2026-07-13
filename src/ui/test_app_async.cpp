#include "test_app_internal.hpp"

namespace cgpui::detail {

bool TestAppState::complete_task(TaskId id) {
  return runtime.complete_task(id);
}

void TestAppState::drain_task_completions() {
  runtime.drain_task_completions();
}

} // namespace cgpui::detail

namespace cgpui {

bool TestApp::complete_task(TaskId id) const {
  return state_->complete_task(id);
}

void TestApp::drain_task_completions() const {
  state_->drain_task_completions();
}

} // namespace cgpui
