#include "cgpui/ui/test_context.hpp"

#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

WindowRuntimeId TestContextCapability::runtime_id() const {
  return context_->window_runtime_id;
}

ViewId TestContextCapability::view_id() const {
  return context_->view_id;
}

int TestContextCapability::frame_index() const {
  return context_->frame_index;
}

ViewInputState TestContextCapability::input_state() const {
  return context_->runtime.input_state();
}

std::optional<EventRoute> TestContextCapability::current_event_route() const {
  return context_->current_event_route();
}

std::optional<ActionDispatchResult>
TestContextCapability::last_action_dispatch() const {
  return context_->last_action_dispatch();
}

InvalidationState TestContextCapability::invalidation_state() const {
  return context_->invalidation_state();
}

void TestContextCapability::clear_invalidation() const {
  context_->clear_invalidation();
}

RuntimeDiagnosticsSnapshot TestContextCapability::diagnostics_snapshot()
    const {
  return context_->diagnostics_snapshot();
}

std::span<const PlatformDiagnosticEvent>
TestContextCapability::platform_diagnostics() const {
  return context_->platform_diagnostics();
}

TestContextCapability WindowRuntimeContext::test_context() const {
  return TestContextCapability(*this);
}

} // namespace cgpui
