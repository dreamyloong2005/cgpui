#include "dx12_texture_renderer_internal.hpp"
#include "dx12_command_observation_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_texture_upload_frame_internal.hpp"
#include "dx12_frame_diagnostics_internal.hpp"
#include "dx12_glyph_native_recording_internal.hpp"
#include "dx12_glyph_work_observer_internal.hpp"
#include <algorithm>

namespace cgpui {
namespace {
Result<void> realize_texture(
    Dx12State& state, const Dx12Texture& texture, Dx12NativeTexture& native, bool glyph) {
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
  if (glyph && state.texture_renderer->glyph_native->observer)
    state.texture_renderer->glyph_native->observer->record({.kind = Dx12TextureCopyKind::allocation,
        .target = resource.Get(), .payload_bytes = std::size_t(texture.width) * texture.height});
  auto srv = state.srv_pool->allocate(*state.device.Get(), 1);
  if (!srv) return std::unexpected(srv.error());
  D3D12_SHADER_RESOURCE_VIEW_DESC view{};
  view.Format = texture.format;
  view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  view.Texture2D.MipLevels = 1;
  state.device->CreateShaderResourceView(resource.Get(), &view, srv->cpu());
  dx12_observe_command(state, {.kind = Dx12CommandEventKind::descriptor_write,
      .object = resource.Get(), .object2 = srv->heap(), .a = srv->cpu().ptr, .b = srv->gpu().ptr});
  native.resource = std::move(resource);
  native.srv = std::move(*srv);
  dx12_observe_glyph_lifetime(state);
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
    Dx12State& state, const Dx12TextureHandle& texture, bool glyph) {
  auto& renderer = *state.texture_renderer;
  if (std::ranges::find(renderer.retained, texture) == renderer.retained.end())
    renderer.retained.push_back(texture);
  auto [entry, inserted] = renderer.native_textures.try_emplace(texture.get());
  auto& native = entry->second;
  if (inserted) native.source = texture;
  if (native.uploaded || std::ranges::find(renderer.pending_uploads, &native) != renderer.pending_uploads.end()) {
    if (glyph) renderer.glyph_native->pin(native); // Exact hit still records a real SRV-slot dependency.
    return &native;
  }
  if (renderer.pending_uploads.size() == renderer.pending_uploads.max_size())
    return std::unexpected(Error{.code = ErrorCode::invalid_argument, .message = "DX12 pending upload marker capacity overflow"});
  renderer.pending_uploads.reserve(renderer.pending_uploads.size() + 1);
  if (glyph) {
    auto prepared = dx12_prepare_glyph_target(state, texture, native);
    if (!prepared) return std::unexpected(prepared.error());
    if (auto realized = realize_texture(state, *texture, native, true); !realized)
      return std::unexpected(realized.error());
    if (auto staged = dx12_stage_glyph_target(state, *texture, *prepared); !staged)
      return std::unexpected(staged.error());
    dx12_record_glyph_target(state, native, *prepared);
  } else {
    if (auto realized = realize_texture(state, *texture, native, false); !realized)
      return std::unexpected(realized.error());
    if (auto uploaded = record_upload(state, *texture, native); !uploaded)
      return std::unexpected(uploaded.error());
  }
  renderer.pending_uploads.push_back(&native); // Capacity was reserved before target commands.
  return &native;
}
} // namespace cgpui
