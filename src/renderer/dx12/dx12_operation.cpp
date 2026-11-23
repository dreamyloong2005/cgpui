#include "dx12_operation_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
Dx12OperationGuard::Dx12OperationGuard(Dx12State& state)
    : lock_(state.operation_mutex), state_(state) { ++state_.operation_depth; }
Dx12OperationGuard::~Dx12OperationGuard() { --state_.operation_depth; }
} // namespace cgpui
