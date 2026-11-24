#include "dx12_texture_region_upload_internal.hpp"
#include "dx12_glyph_upload_plan_internal.hpp"
#include "dx12_texture_upload_frame_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
Result<Dx12TextureUploadSlice> dx12_upload_glyph_region(
    Dx12State& state, const Dx12GlyphUploadChunk& chunk) {
  if (auto valid = dx12_validate_texture_upload_state(state); !valid) return std::unexpected(valid.error());
  if (chunk.batch.uploads.size() != 1)
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 glyph region requires one validated region"});
  const auto& region = chunk.batch.uploads[0];
  if (region.byte_offset > chunk.batch.alpha.size() ||
      region.byte_size > chunk.batch.alpha.size() - region.byte_offset)
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 glyph region chunk offset escapes its owned alpha"});
  const Dx12TextureSourceView view{region.width, region.height, region.stride, DXGI_FORMAT_R8_UNORM,
      std::span<const std::uint8_t>{chunk.batch.alpha}.subspan(region.byte_offset, region.byte_size)};
  return state.texture_uploads->upload_region(*state.device.Get(), view);
}
} // namespace cgpui
