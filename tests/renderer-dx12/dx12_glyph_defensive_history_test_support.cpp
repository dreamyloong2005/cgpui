#include "dx12_glyph_defensive_history_test_support.hpp"
#include "dx12_glyph_cpu_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_record_validation_internal.hpp"
#include "../../src/renderer/dx12/dx12_glyph_recording_scope_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_upload_frame_internal.hpp"
#include <cstring>

namespace cgpui_test {
GlyphDefensiveHistory::GlyphDefensiveHistory() {
  glyph_require(bool(lineage.ensure_page(0, 16, 16)), "defensive family");
}
cgpui::Dx12TextureHandle GlyphDefensiveHistory::append(
    unsigned id, unsigned width, unsigned height, unsigned y) {
  auto record = glyph_cpu_record(id, 0, 0, y, width, height, width + 3);
  pages[0].entries.push_back({.key = record.key, .page_index = 0, .atlas_bounds = record.atlas_bounds});
  const auto valid = cgpui::dx12_validate_glyph_record(record, pages[0], 16, 16);
  glyph_require(bool(valid), "defensive record must pass real source validation");
  const auto index = records.size();
  glyph_require(bool(lineage.prepare_record(0, index)), "defensive mapping");
  for (unsigned row = 0; row < height; ++row)
    std::memcpy(alpha.data() + std::size_t(y + row) * 16,
        record.alpha.data() + std::size_t(row) * record.stride, width);
  records.push_back(std::move(record));
  lineage.commit_record(0, index);
  auto snapshot = std::make_shared<cgpui::Dx12Texture>(cgpui::Dx12Texture{
      .width = 16, .height = 16, .stride = 16, .format = DXGI_FORMAT_R8_UNORM, .pixels = alpha});
  glyph_require(bool(lineage.seal_snapshot(0, snapshot)), "defensive seal");
  return snapshot;
}
cgpui::Dx12GlyphTargetInputs GlyphDefensiveHistory::inputs() const {
  return {&lineage, records, pages};
}
cgpui::Result<void> glyph_defensive_submit(cgpui::Dx12State& state,
    const cgpui::Dx12TextureHandle& snapshot, const GlyphDefensiveHistory& history) {
  using namespace cgpui;
  Dx12GlyphRecordingScope scope(state);
  if (!scope.entered()) return std::unexpected(dx12_error(E_FAIL, "defensive scope"));
  if (auto idle = dx12_wait_idle(state); !idle) return idle;
  dx12_begin_texture_frame(state);
  HRESULT hr = state.allocator->Reset();
  if (SUCCEEDED(hr)) hr = state.commands->Reset(state.allocator.Get(), nullptr);
  if (FAILED(hr)) return std::unexpected(dx12_error(hr, "defensive Reset"));
  scope.opened_after_reset();
  auto& renderer = *state.texture_renderer;
  renderer.glyph_native->begin_recording(history.lineage);
  if (auto ready = dx12_begin_texture_upload_frame(state); !ready) return ready;
  renderer.retained.push_back(snapshot);
  auto [entry, added] = renderer.native_textures.try_emplace(snapshot.get());
  auto& native = entry->second;
  if (added) native.source = snapshot;
  renderer.pending_uploads.reserve(renderer.pending_uploads.size() + 1);
  auto prepared = dx12_prepare_glyph_target(state, snapshot, native, history.inputs());
  if (!prepared) return std::unexpected(prepared.error());
  if (!native.resource) {
    D3D12_HEAP_PROPERTIES heap{}; heap.Type = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_RESOURCE_DESC desc{};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; desc.Width = snapshot->width;
    desc.Height = snapshot->height; desc.DepthOrArraySize = 1; desc.MipLevels = 1;
    desc.Format = snapshot->format; desc.SampleDesc.Count = 1;
    Dx12Ptr<ID3D12Resource> resource;
    hr = state.device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resource));
    if (FAILED(hr)) return std::unexpected(dx12_error(hr, "defensive target resource"));
    auto lease = state.srv_pool->allocate(*state.device.Get(), 1);
    if (!lease) return std::unexpected(lease.error());
    native.resource = std::move(resource); native.srv = std::move(*lease);
    // No descriptor binding occurs in this copy/readback control; no fake allocation event is emitted.
  }
  if (auto staged = dx12_stage_glyph_target(state, *snapshot, *prepared); !staged) return staged;
  dx12_record_glyph_target(state, native, *prepared);
  renderer.pending_uploads.push_back(&native);
  hr = scope.close();
  if (FAILED(hr)) return std::unexpected(dx12_error(hr, "defensive Close"));
  ID3D12CommandList* lists[] = {state.commands.Get()};
  state.queue->ExecuteCommandLists(1, lists);
  scope.executed();
  if (auto idle = dx12_wait_idle(state); !idle) return idle;
  dx12_commit_texture_uploads(state);
  return dx12_check_debug_errors(state); // G already proven; debug acceptance cannot invent completion.
}
} // namespace cgpui_test
