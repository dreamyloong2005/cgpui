#include "dx12_glyph_fallback_controls_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_recording_scope_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_upload_frame_internal.hpp"
#include <cstdlib>
#include <utility>

namespace cgpui_test {
bool glyph_fallback_enable_debug() { return _putenv_s("CGPUI_DX12_DEBUG", "1") == 0; }
cgpui::Result<void> glyph_fallback_submit(cgpui::Dx12State& state,
    const cgpui::Dx12TextureHandle& snapshot, const cgpui::Dx12GlyphLineage& recording_lineage,
    const cgpui::Dx12GlyphTargetInputs& inputs) {
  using namespace cgpui;
  Dx12GlyphRecordingScope scope(state);
  if (!scope.entered()) return std::unexpected(dx12_error(E_FAIL, "fallback fixture scope"));
  if (auto idle = dx12_wait_idle(state); !idle) return idle;
  dx12_begin_texture_frame(state);
  HRESULT hr = state.allocator->Reset();
  if (SUCCEEDED(hr)) hr = state.commands->Reset(state.allocator.Get(), nullptr);
  if (FAILED(hr)) return std::unexpected(dx12_error(hr, "fallback fixture Reset"));
  scope.opened_after_reset();
  auto& renderer = *state.texture_renderer;
  renderer.glyph_native->begin_recording(recording_lineage);
  if (auto ready = dx12_begin_texture_upload_frame(state); !ready) return ready;
  renderer.retained.push_back(snapshot);
  auto [entry, added] = renderer.native_textures.try_emplace(snapshot.get());
  auto& native = entry->second;
  if (added) native.source = snapshot;
  if (native.resource || native.uploaded) return std::unexpected(Error{
      .code = ErrorCode::invalid_argument, .message = "fallback fixture requires fresh target"});
  renderer.pending_uploads.reserve(renderer.pending_uploads.size() + 1);
  auto prepared = dx12_prepare_glyph_target(state, snapshot, native, inputs);
  if (!prepared) return std::unexpected(prepared.error());
  D3D12_HEAP_PROPERTIES heap{}; heap.Type = D3D12_HEAP_TYPE_DEFAULT;
  D3D12_RESOURCE_DESC descriptor{};
  descriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  descriptor.Width = snapshot->width; descriptor.Height = snapshot->height;
  descriptor.DepthOrArraySize = 1; descriptor.MipLevels = 1;
  descriptor.Format = snapshot->format; descriptor.SampleDesc.Count = 1;
  Dx12Ptr<ID3D12Resource> resource;
  hr = state.device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &descriptor,
      D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resource));
  if (FAILED(hr)) return std::unexpected(dx12_error(hr, "fallback fixture target"));
  auto lease = state.srv_pool->allocate(*state.device.Get(), 1);
  if (!lease) return std::unexpected(lease.error());
  native.resource = std::move(resource); native.srv = std::move(*lease);
  // Copy/readback only: unused lease is real, no SRV binding or invented allocation observation.
  if (auto staged = dx12_stage_glyph_target(state, *snapshot, *prepared); !staged) return staged;
  dx12_record_glyph_target(state, native, *prepared);
  renderer.pending_uploads.push_back(&native);
  hr = scope.close();
  if (FAILED(hr)) return std::unexpected(dx12_error(hr, "fallback fixture Close"));
  ID3D12CommandList* lists[] = {state.commands.Get()};
  state.queue->ExecuteCommandLists(1, lists);
  scope.executed(); // Immediate no-throw submitted ownership, never synthetic completion.
  if (auto idle = dx12_wait_idle(state); !idle) return idle;
  dx12_commit_texture_uploads(state);
  return dx12_check_debug_errors(state);
}
} // namespace cgpui_test
