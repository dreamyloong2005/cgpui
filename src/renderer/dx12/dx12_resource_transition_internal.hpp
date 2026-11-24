#pragma once

#include "dx12_native_internal.hpp"

namespace cgpui {
void dx12_transition(ID3D12GraphicsCommandList& commands, ID3D12Resource& resource,
                     D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
} // namespace cgpui
