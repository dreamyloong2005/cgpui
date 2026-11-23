#pragma once

#include "dx12_native_internal.hpp"
#include "cgpui/renderer/planning/rect_geometry_types.hpp"
#include <cstddef>
#include <span>

namespace cgpui {
struct Dx12State;
// Callers validate their vertex layout, finite geometry and index ranges.
// Indexed primitives share the framebuffer-extent root signature and retain
// their upload buffers in the existing submitted-frame lifetime owner.
Result<void> dx12_record_indexed_draw(
    Dx12State& state, ID3D12PipelineState& pipeline,
    std::span<const std::byte> vertex_bytes, UINT vertex_stride,
    std::span<const std::uint32_t> index_data, std::span<const RectDrawRange> draws);
} // namespace cgpui
