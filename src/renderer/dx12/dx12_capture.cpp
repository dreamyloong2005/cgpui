#include "dx12_capture_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_buffers_internal.hpp"
#include <cstring>

namespace cgpui {
Result<void> dx12_prepare_capture(Dx12State& state) {
  const auto descriptor = state.targets[0]->GetDesc();
  UINT64 bytes = 0;
  state.device->GetCopyableFootprints(&descriptor, 0, 1, 0,
      &state.capture_layout, nullptr, nullptr, &bytes);
  auto buffer = dx12_create_buffer(*state.device.Get(), bytes, D3D12_HEAP_TYPE_READBACK);
  if (!buffer) return std::unexpected(buffer.error());
  state.readback = std::move(*buffer);
  return {};
}

void dx12_record_capture(Dx12State& state, ID3D12Resource& target) {
  D3D12_TEXTURE_COPY_LOCATION source{};
  source.pResource = &target;
  source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  D3D12_TEXTURE_COPY_LOCATION destination{};
  destination.pResource = state.readback.Get();
  destination.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  destination.PlacedFootprint = state.capture_layout;
  state.commands->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
}

Result<RendererFramePixels> dx12_read_capture(Dx12State& state) {
  const auto pitch = state.capture_layout.Footprint.RowPitch;
  const auto offset = static_cast<SIZE_T>(state.capture_layout.Offset);
  // GetCopyableFootprints does not allocate trailing padding after the final
  // row. Mapping a full pitch for that row exceeds non-aligned-width buffers.
  const D3D12_RANGE read{offset,
      offset + SIZE_T(pitch) * (state.height - 1) + SIZE_T(state.width) * 4};
  RendererFramePixels pixels{.width = state.width, .height = state.height,
                            .encoding = RendererFramePixelEncoding::linear};
  // Allocate before mapping: an allocation exception must not leave it mapped.
  pixels.rgba8.resize(std::size_t(state.width) * state.height * 4);
  void* mapped = nullptr;
  const HRESULT result = state.readback->Map(0, &read, &mapped);
  if (FAILED(result)) return std::unexpected(dx12_error(result, "capture mapping"));
  const auto* source = static_cast<const unsigned char*>(mapped) + offset;
  const auto row = std::size_t(state.width) * 4;
  for (UINT y = 0; y != state.height; ++y) {
    std::memcpy(pixels.rgba8.data() + y * row, source + std::size_t(y) * pitch, row);
  }
  const D3D12_RANGE written{0, 0};
  state.readback->Unmap(0, &written);
  return pixels; // Caller owns the candidate until all successful-frame checks pass.
}
} // namespace cgpui
