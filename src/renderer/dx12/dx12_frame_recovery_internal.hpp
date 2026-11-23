#pragma once

#include "dx12_native_internal.hpp"

namespace cgpui {
struct Dx12State;
Result<void> dx12_prepare_frame(Dx12State& state);
Result<void> dx12_handle_frame_failure(Dx12State& state, Error error);
Result<void> dx12_resize_recovering(Dx12State& state, Size size, DpiScale scale);
} // namespace cgpui
