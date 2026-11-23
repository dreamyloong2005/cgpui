#pragma once

#include "dx12_native_internal.hpp"
#include <cstdint>

namespace cgpui {
struct Dx12State;

HRESULT dx12_open_adapter_device(Dx12State& state);
Result<void> dx12_select_initial_adapter(Dx12State& state);
Result<void> dx12_select_pinned_adapter(Dx12State& state, std::uint64_t luid);
// Recovery never consults initial adapter selection or chooses a fallback.
Result<void> dx12_create_device_on_adapter(Dx12State& state, std::uint64_t luid);
} // namespace cgpui
