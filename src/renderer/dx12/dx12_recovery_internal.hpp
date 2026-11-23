#pragma once

#include "dx12_native_internal.hpp"

namespace cgpui {
struct Dx12State;

// false: no confirmed loss; true: complete rebuild; error: one bounded attempt
// failed. A confirmed loss remains pending so a later public call may try again.
Result<bool> dx12_recover_removed_device(Dx12State& state);
} // namespace cgpui
