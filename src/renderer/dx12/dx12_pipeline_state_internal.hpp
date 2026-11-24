#pragma once

#include "dx12_native_internal.hpp"
#include "dx12_initialization_failure_internal.hpp"

namespace cgpui {
struct Dx12State;
Result<void> dx12_create_pipeline(Dx12State& state, Dx12InitializationFailureClass* failure = nullptr);
// Fixed-state contract shared by all opaque-surface primitive pipelines.
D3D12_GRAPHICS_PIPELINE_STATE_DESC dx12_pipeline_defaults(ID3D12RootSignature* signature);
} // namespace cgpui
