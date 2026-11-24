#pragma once

#include "dx12_texture_upload_internal.hpp"

namespace cgpui {
struct Dx12State;
// Shared private gate for all full/region writes and pool rewind.
Result<void> dx12_validate_texture_upload_state(const Dx12State& state);
// Called only after submission completion is proven; never from begin_frame.
Result<void> dx12_begin_texture_upload_frame(Dx12State& state);
Result<Dx12TextureUploadSlice> dx12_upload_texture(Dx12State& state, const Dx12Texture& texture);
} // namespace cgpui
