#pragma once

#include "dx12_texture_upload_internal.hpp"

namespace cgpui {
struct Dx12State;
// Called only after submission completion is proven; never from begin_frame.
Result<void> dx12_begin_texture_upload_frame(Dx12State& state);
Result<Dx12TextureUploadSlice> dx12_upload_texture(Dx12State& state, const Dx12Texture& texture);
} // namespace cgpui
