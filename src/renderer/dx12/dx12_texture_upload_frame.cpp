#include "dx12_texture_upload_frame_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
namespace {
Result<void> writable(const Dx12State& state) {
  if (!state.device || state.submitted || state.failed || state.recovery_pending ||
      FAILED(state.removal_reason)) return std::unexpected(Error{
          .code = ErrorCode::invalid_argument,
          .message = "DX12 texture uploads require a live, idle device generation"});
  return {};
}
} // namespace

Result<void> dx12_begin_texture_upload_frame(Dx12State& state) {
  if (auto valid = writable(state); !valid) return valid;
  state.texture_uploads->begin_frame();
  return {};
}

Result<Dx12TextureUploadSlice> dx12_upload_texture(Dx12State& state, const Dx12Texture& texture) {
  if (auto valid = writable(state); !valid) return std::unexpected(valid.error());
  return state.texture_uploads->upload(*state.device.Get(), texture);
}
} // namespace cgpui
