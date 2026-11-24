#pragma once
#include "dx12_native_internal.hpp"

namespace cgpui {
struct Dx12State;
Result<bool> dx12_enable_debug();
Result<void> dx12_check_debug_errors(Dx12State& state);
}
