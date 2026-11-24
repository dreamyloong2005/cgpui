#include "dx12_glyph_native_recording_internal.hpp"
#include "dx12_texture_renderer_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_frame_diagnostics_internal.hpp"
#include "dx12_glyph_work_observer_internal.hpp"
#include <cassert>

namespace cgpui {
void dx12_record_glyph_target(
    Dx12State& state, Dx12NativeTexture& native, Dx12GlyphPreparedTarget& prepared) noexcept {
  auto& owner = *state.texture_renderer->glyph_native;
  auto* observer = owner.observer;
  auto* work = owner.work_observer;
  if (prepared.regions) {
    dx12_transition(*state.commands.Get(), *prepared.base.resource.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE);
    if (work) work->record({.kind = Dx12GlyphWorkKind::barrier, .resource = prepared.base.resource.Get(),
        .before = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, .after = D3D12_RESOURCE_STATE_COPY_SOURCE});
    state.commands->CopyResource(native.resource.Get(), prepared.base.resource.Get());
    if (observer) observer->record({.kind = Dx12TextureCopyKind::clone,
        .source = prepared.base.resource.Get(), .target = native.resource.Get(),
        .payload_bytes = prepared.regions->full_bytes});
    dx12_transition(*state.commands.Get(), *prepared.base.resource.Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    if (work) work->record({.kind = Dx12GlyphWorkKind::barrier, .resource = prepared.base.resource.Get(),
        .before = D3D12_RESOURCE_STATE_COPY_SOURCE, .after = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE});
  }
  for (std::size_t index = 0; index < prepared.staging.size(); ++index) {
    const auto& slice = prepared.staging[index];
    UINT x = 0, y = 0;
    if (prepared.regions) {
      const auto& region = prepared.regions->chunks[index].batch.uploads[0];
      x = static_cast<UINT>(region.atlas_bounds.origin.x);
      y = static_cast<UINT>(region.atlas_bounds.origin.y);
    }
    D3D12_TEXTURE_COPY_LOCATION source{};
    source.pResource = slice.resource; source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    source.PlacedFootprint = slice.footprint;
    D3D12_TEXTURE_COPY_LOCATION target{};
    target.pResource = native.resource.Get(); target.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    state.commands->CopyTextureRegion(&target, x, y, 0, &source, nullptr);
    if (observer) observer->record({.kind = prepared.regions ? Dx12TextureCopyKind::region_upload : Dx12TextureCopyKind::full_upload,
        .source = slice.resource, .target = native.resource.Get(), .payload_bytes = slice.payload_bytes,
        .x = x, .y = y, .width = slice.footprint.Footprint.Width, .height = slice.footprint.Footprint.Height,
        .row_pitch = slice.footprint.Footprint.RowPitch, .offset = slice.footprint.Offset});
    if (state.active_diagnostics) state.active_diagnostics->uploaded(true, slice.payload_bytes);
  }
  dx12_transition(*state.commands.Get(), *native.resource.Get(),
      D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
  if (work) work->record({.kind = Dx12GlyphWorkKind::barrier, .resource = native.resource.Get(),
      .before = D3D12_RESOURCE_STATE_COPY_DEST, .after = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE});
  if (prepared.snapshot) owner.publish_candidate(*prepared.snapshot, *native.resource.Get());
}
} // namespace cgpui
