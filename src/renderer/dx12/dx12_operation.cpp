#include "dx12_operation_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_recovery_work_internal.hpp"

namespace cgpui {
Dx12OperationGuard::Dx12OperationGuard(Dx12State& state)
    : lock_(state.operation_mutex), state_(state) { ++state_.operation_depth; }
Dx12OperationGuard::~Dx12OperationGuard() {
  const bool idle = --state_.operation_depth == 0;
  const auto lease = state_.recovery_lease;
  const auto token = state_.recovery_work_token;
  lock_.unlock();
  // No peer work while holding this gate, and only the outermost entry retries
  // an explicitly requested retirement. A deferred callback never self-spins.
  if (idle && token) dx12_post_recovery_work(token, lease);
}
} // namespace cgpui
