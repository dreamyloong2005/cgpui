#pragma once

#include "dx12_native_internal.hpp"
#include "cgpui/renderer/planning/box_shadow_geometry.hpp"

namespace cgpui {
struct Dx12State;
bool dx12_finite_shadow_geometry(const BoxShadowGeometry& geometry);
Result<void> dx12_create_shadow_pipeline(Dx12State& state);
Result<void> dx12_record_shadow_geometry(Dx12State& state, const BoxShadowGeometry& geometry);
} // namespace cgpui
