#pragma once
#include "dx12_texture_upload_internal.hpp"

namespace cgpui {
struct Dx12GlyphUploadChunk;
Result<Dx12TextureUploadSlice> dx12_upload_glyph_region(Dx12State&, const Dx12GlyphUploadChunk&);
} // namespace cgpui
