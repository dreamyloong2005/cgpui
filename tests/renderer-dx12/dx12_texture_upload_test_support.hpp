#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "dx12_diagnostics_support.hpp"
#include "dx12_sync_resource_sentinel.hpp"
#include "../../src/renderer/dx12/dx12_texture_upload_frame_internal.hpp"
#include "../../src/renderer/dx12/dx12_frame_internal.hpp"
#include "../../src/renderer/dx12/dx12_surface_internal.hpp"
#include <cstring>
#include <tuple>

namespace cgpui_test {
inline constexpr std::size_t texture_page_bytes = 256 * 1024;
inline cgpui::Dx12Texture staging_texture(UINT width, UINT height, DXGI_FORMAT format,
                                         UINT padding = 0, unsigned seed = 1) {
  const UINT row = width * (format == DXGI_FORMAT_R8_UNORM ? 1 : 4);
  cgpui::Dx12Texture texture{.width = width, .height = height, .stride = row + padding,
      .format = format, .pixels = std::vector<std::uint8_t>(std::size_t(row + padding) * (height - 1) + row, 0xe7)};
  for (UINT y = 0; y != height; ++y) for (UINT x = 0; x != row; ++x)
    texture.pixels[std::size_t(y) * texture.stride + x] = static_cast<unsigned char>((seed + x * 13 + y * 31) % 256);
  return texture;
}
inline auto texture_upload_stats(const cgpui::Dx12TextureUploads& uploads) {
  const auto value = uploads.stats();
  return std::make_tuple(value.page_count, value.capacity_bytes, value.used_bytes,
                        value.allocations_this_frame, value.total_allocations);
}
inline bool staging_bytes(const cgpui::Dx12TextureUploadSlice& slice, const cgpui::Dx12Texture& source) {
  const auto& footprint = slice.footprint;
  const std::size_t row = source.width * (source.format == DXGI_FORMAT_R8_UNORM ? 1U : 4U);
  if (!slice.resource || footprint.Offset % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT ||
      footprint.Footprint.RowPitch % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT || footprint.Footprint.RowPitch < row ||
      footprint.Footprint.Width != source.width || footprint.Footprint.Height != source.height ||
      footprint.Footprint.Depth != 1 || footprint.Footprint.Format != source.format ||
      slice.payload_bytes != row * source.height) return false;
  const auto end = footprint.Offset + std::size_t(footprint.Footprint.RowPitch) * (source.height - 1) + row;
  if (end > slice.resource->GetDesc().Width) return false;
  void* mapped = nullptr;
  const D3D12_RANGE read{static_cast<SIZE_T>(footprint.Offset), static_cast<SIZE_T>(end)};
  if (FAILED(slice.resource->Map(0, &read, &mapped))) return false;
  bool equal = true;
  for (UINT y = 0; y != source.height; ++y)
    equal &= std::memcmp(static_cast<unsigned char*>(mapped) + footprint.Offset + std::size_t(y) * footprint.Footprint.RowPitch,
                         source.pixels.data() + std::size_t(y) * source.stride, row) == 0;
  const D3D12_RANGE no_writes{0, 0};
  slice.resource->Unmap(0, &no_writes);
  return equal;
}
inline std::shared_ptr<cgpui::Dx12State> texture_upload_state(Dx12Window* window = nullptr) {
  auto state = std::make_shared<cgpui::Dx12State>();
  if (window && (!window->hwnd || !cgpui::dx12_configure_surface(*state, window->descriptor()))) return {};
  if (!cgpui::dx12_create_device(*state)) return {};
  if (window && (!cgpui::dx12_create_pipeline(*state) || !cgpui::dx12_create_swapchain(*state))) return {};
  return state;
}
} // namespace cgpui_test
