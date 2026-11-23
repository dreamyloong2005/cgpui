#include "dx12_texture_renderer_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_texture_upload_frame_internal.hpp"
#include "dx12_frame_diagnostics_internal.hpp"
#include <algorithm>

namespace cgpui {
namespace {
Result<void> realize_texture(
    Dx12State& state, const Dx12Texture& texture, Dx12NativeTexture& native) {
  if (native.resource) return {};
  D3D12_HEAP_PROPERTIES heap{};
  heap.Type = D3D12_HEAP_TYPE_DEFAULT;
  D3D12_RESOURCE_DESC descriptor{};
  descriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  descriptor.Width = texture.width;
  descriptor.Height = texture.height;
  descriptor.DepthOrArraySize = 1;
  descriptor.MipLevels = 1;
  descriptor.Format = texture.format;
  descriptor.SampleDesc.Count = 1;
  Dx12Ptr<ID3D12Resource> resource;
  HRESULT result = state.device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE,
      &descriptor, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resource));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "texture allocation"));
  D3D12_DESCRIPTOR_HEAP_DESC heap_descriptor{};
  heap_descriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  heap_descriptor.NumDescriptors = 1;
  heap_descriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  Dx12Ptr<ID3D12DescriptorHeap> descriptors;
  result = state.device->CreateDescriptorHeap(&heap_descriptor, IID_PPV_ARGS(&descriptors));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "texture descriptor heap"));
  D3D12_SHADER_RESOURCE_VIEW_DESC view{};
  view.Format = texture.format;
  view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  view.Texture2D.MipLevels = 1;
  state.device->CreateShaderResourceView(resource.Get(), &view,
      descriptors->GetCPUDescriptorHandleForHeapStart());
  native.resource = std::move(resource);
  native.descriptors = std::move(descriptors);
  return {};
}

Result<void> record_upload(
    Dx12State& state, const Dx12Texture& texture, Dx12NativeTexture& native) {
  auto upload = dx12_upload_texture(state, texture);
  if (!upload) return std::unexpected(upload.error());
  D3D12_TEXTURE_COPY_LOCATION source{};
  source.pResource = upload->resource;
  source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  source.PlacedFootprint = upload->footprint;
  D3D12_TEXTURE_COPY_LOCATION target{};
  target.pResource = native.resource.Get();
  target.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  state.commands->CopyTextureRegion(&target, 0, 0, 0, &source, nullptr);
  if (state.active_diagnostics) state.active_diagnostics->uploaded(
      texture.format == DXGI_FORMAT_R8_UNORM, upload->payload_bytes);
  dx12_transition(*state.commands.Get(), *native.resource.Get(),
      D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
  return {};
}
} // namespace

Result<Dx12NativeTexture*> dx12_prepare_texture(
    Dx12State& state, const Dx12TextureHandle& texture) {
  auto& renderer = *state.texture_renderer;
  if (std::ranges::find(renderer.retained, texture) == renderer.retained.end()) {
    renderer.retained.push_back(texture);
  }
  auto [entry, inserted] = renderer.native_textures.try_emplace(texture.get());
  auto& native = entry->second;
  if (inserted) native.source = texture;
  if (native.uploaded || std::ranges::find(renderer.pending_uploads, &native) !=
      renderer.pending_uploads.end()) return &native;
  if (auto realized = realize_texture(state, *texture, native); !realized) {
    return std::unexpected(realized.error());
  }
  if (auto uploaded = record_upload(state, *texture, native); !uploaded) {
    return std::unexpected(uploaded.error());
  }
  renderer.pending_uploads.push_back(&native);
  return &native;
}

void dx12_begin_texture_frame(Dx12State& state) {
  auto& renderer = *state.texture_renderer;
  renderer.pending_uploads.clear();
  renderer.retained.clear();
  std::erase_if(renderer.native_textures, [](const auto& entry) {
    return entry.second.source.expired();
  });
}
void dx12_commit_texture_uploads(Dx12State& state) {
  for (auto* native : state.texture_renderer->pending_uploads) native->uploaded = true;
  state.texture_renderer->pending_uploads.clear();
}
} // namespace cgpui
