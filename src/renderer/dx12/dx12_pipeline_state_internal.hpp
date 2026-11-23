#pragma once

#include "dx12_native_internal.hpp"

namespace cgpui {
// Fixed-state contract shared by all opaque-surface primitive pipelines.
D3D12_GRAPHICS_PIPELINE_STATE_DESC dx12_pipeline_defaults(ID3D12RootSignature* signature);
} // namespace cgpui
