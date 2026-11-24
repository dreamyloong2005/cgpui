#include "dx12_command_reuse_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_buffers_internal.hpp"
#include <cstring>

namespace cgpui_test {
using namespace cgpui;
void ReuseQueueTail::prepare(Dx12State& state, ID3D12Resource& target, std::uint64_t epoch) noexcept {
  ready_ = false;
  if (submission_pending()) { failed = true; return; } // Never release unproven prior tail owners.
  retired(false);
  parent = epoch; completed_parent = ticket = 0; target_identity = &target;
  try { if (!prepare_checked(state, target)) { failed = true; return; } }
  catch (...) { failed = true; return; }
  ready_ = true; // Preparation is NOT execution; the production forwarding owner mints that fact.
}
Result<void> ReuseQueueTail::prepare_checked(Dx12State& state, ID3D12Resource& target) {
  target_ = &target;
  const auto desc = target.GetDesc();
  if (desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM || desc.Width != state.width || desc.Height != state.height)
    return std::unexpected(dx12_error(E_INVALIDARG, "reuse tail actual target"));
  UINT rows = 0; UINT64 row_bytes = 0;
  state.device->GetCopyableFootprints(&desc, 0, 1, 0, &layout_, &rows, &row_bytes, &bytes_);
  auto buffer = dx12_create_buffer(*state.device.Get(), static_cast<std::size_t>(bytes_), D3D12_HEAP_TYPE_READBACK);
  if (!buffer) return std::unexpected(buffer.error());
  buffer_ = std::move(*buffer);
  pixels = {.width = state.width, .height = state.height, .rgba8 = std::vector<std::uint8_t>(std::size_t(state.width) * state.height * 4)};
  HRESULT hr = state.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator_));
  if (SUCCEEDED(hr)) hr = state.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
      allocator_.Get(), nullptr, IID_PPV_ARGS(&commands_));
  if (FAILED(hr)) return std::unexpected(dx12_error(hr, "reuse verifier create"));
  dx12_transition(*commands_.Get(), target, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_SOURCE);
  D3D12_TEXTURE_COPY_LOCATION source{}, destination{};
  source.pResource = &target; source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  destination.pResource = buffer_.Get(); destination.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  destination.PlacedFootprint = layout_;
  commands_->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
  dx12_transition(*commands_.Get(), target, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PRESENT);
  hr = commands_->Close();
  if (FAILED(hr)) return std::unexpected(dx12_error(hr, "reuse verifier Close"));
  return {};
}
ID3D12CommandList* ReuseQueueTail::prepared_list() const noexcept { return ready_ ? commands_.Get() : nullptr; }
void ReuseQueueTail::completed(Dx12State&, std::uint64_t epoch, std::uint64_t fence) noexcept {
  if (!submission_pending() || epoch != parent || !fence) { failed = true; return; }
  ready_ = false;
  void* mapped = nullptr; const D3D12_RANGE read{0, static_cast<SIZE_T>(bytes_)};
  if (FAILED(buffer_->Map(0, &read, &mapped))) { failed = true; return; }
  for (UINT y = 0; y < pixels.height; ++y)
    std::memcpy(pixels.rgba8.data() + std::size_t(y) * pixels.width * 4,
        static_cast<const std::uint8_t*>(mapped) + layout_.Offset + std::size_t(y) * layout_.Footprint.RowPitch,
        std::size_t(pixels.width) * 4);
  const D3D12_RANGE no_write{0, 0}; buffer_->Unmap(0, &no_write);
  completed_parent = epoch; ticket = fence;
  // The tail has no future Execute: release its own parent first, before target/ResizeBuffers.
  commands_.Reset(); allocator_.Reset(); buffer_.Reset(); target_.Reset();
}
void ReuseQueueTail::retired(bool removed) noexcept {
  if (submission_pending() && !removed) { failed = true; return; }
  if (removed) failed = true;
  commands_.Reset(); allocator_.Reset(); buffer_.Reset(); target_.Reset(); ready_ = false;
}
} // namespace cgpui_test
