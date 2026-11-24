#pragma once

#include "dx12_native_internal.hpp"
#include "dx12_initialization_failure_internal.hpp"

namespace cgpui {
struct Dx12State;
Result<void> dx12_create_swapchain(Dx12State& state, Dx12InitializationFailureClass* failure = nullptr);
Result<void> dx12_create_targets(Dx12State& state);
Result<void> dx12_resize(Dx12State& state, Size size, DpiScale scale);
} // namespace cgpui
