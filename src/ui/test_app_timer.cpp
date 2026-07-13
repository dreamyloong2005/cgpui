#include "test_app_internal.hpp"

#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/test_context.hpp"

namespace cgpui::detail {

void TestAppState::advance_time(std::uint64_t delta_ms) {
  runtime.advance_time(delta_ms);
}

void TestAppState::run_until_parked() {
  const WindowRuntimeContext runtime_context = runtime.context();
  runtime_context.test_context().run_until_parked();
}

bool TestAppState::cancel_timer(TimerId id) {
  return runtime.cancel_timer(id);
}

} // namespace cgpui::detail

namespace cgpui {

void TestApp::advance_time(std::uint64_t delta_ms) const {
  state_->advance_time(delta_ms);
}

void TestApp::run_until_parked() const {
  state_->run_until_parked();
}

void TestApp::advance_time_until_parked(std::uint64_t delta_ms) const {
  state_->advance_time(delta_ms);
  state_->run_until_parked();
}

bool TestApp::cancel_timer(TimerId id) const {
  return state_->cancel_timer(id);
}

} // namespace cgpui
