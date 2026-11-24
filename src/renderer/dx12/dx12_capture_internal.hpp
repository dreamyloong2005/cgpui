#pragma once

#include "dx12_native_internal.hpp"

namespace cgpui {
struct Dx12State;
Result<void> dx12_prepare_capture(Dx12State& state);
void dx12_record_capture(Dx12State& state, ID3D12Resource& target);
Result<RendererFramePixels> dx12_read_capture(Dx12State& state);
} // namespace cgpui
