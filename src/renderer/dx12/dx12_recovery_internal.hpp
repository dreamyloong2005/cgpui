#pragma once

#include "dx12_native_internal.hpp"

namespace cgpui {
struct Dx12State;

// false: no confirmed loss; true: complete rebuild. renderer_recovery_pending
// means owner retirement is unfinished (no waiting here); all other errors are
// explicit failures. Repeated calls never extend the loss wave's deadline.
Result<bool> dx12_recover_removed_device(Dx12State& state);
} // namespace cgpui
