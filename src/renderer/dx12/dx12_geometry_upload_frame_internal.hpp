#pragma once

#include "dx12_geometry_upload_internal.hpp"

namespace cgpui {
struct Dx12State;
// Called only after submission completion is proven; never from begin_frame.
Result<void> dx12_begin_geometry_upload_frame(Dx12State& state);
Result<Dx12GeometryUploadSlice> dx12_upload_geometry(
    Dx12State& state, const void* data, std::size_t size);
} // namespace cgpui
