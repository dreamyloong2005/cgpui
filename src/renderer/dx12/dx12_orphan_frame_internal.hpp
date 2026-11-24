#pragma once

#include "dx12_native_internal.hpp"
#include <cstdint>

namespace cgpui {
struct Dx12State;
// A frame can retain a healthy native epoch after its facade dies, never rebuild it.
Result<void> dx12_prepare_orphan_present(Dx12State& state, std::uint64_t generation);
Result<void> dx12_fail_orphan_frame(Dx12State& state, Error error);
} // namespace cgpui
