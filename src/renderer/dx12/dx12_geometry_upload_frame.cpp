#include "dx12_geometry_upload_frame_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
namespace {
Result<void> writable(const Dx12State& state) {
  if (!state.device || state.submitted || state.failed || state.recovery_pending ||
      FAILED(state.removal_reason)) return std::unexpected(Error{
          .code = ErrorCode::invalid_argument,
          .message = "DX12 geometry uploads require a live, idle device generation"});
  return {};
}
} // namespace

Result<void> dx12_begin_geometry_upload_frame(Dx12State& state) {
  if (auto valid = writable(state); !valid) return valid;
  state.geometry_uploads->begin_frame();
  return {};
}

Result<Dx12GeometryUploadSlice> dx12_upload_geometry(
    Dx12State& state, const void* data, std::size_t size) {
  if (auto valid = writable(state); !valid) return std::unexpected(valid.error());
  return state.geometry_uploads->upload(*state.device.Get(), data, size);
}
} // namespace cgpui
