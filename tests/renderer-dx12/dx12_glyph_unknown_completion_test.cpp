#include "dx12_glyph_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_synchronization_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;
namespace {
struct HideCompletion final : Dx12SynchronizationApi {
  Dx12NativeSynchronizationApi native;
  HRESULT signal(ID3D12CommandQueue& queue, ID3D12Fence& fence, UINT64 value) noexcept override {
    return native.signal(queue, fence, value); // Real GPU queue ticket, not synthetic completion.
  }
  UINT64 completed(ID3D12Fence&) noexcept override { return 0; }
  HRESULT event(ID3D12Fence&, UINT64, HANDLE) noexcept override { return E_ACCESSDENIED; }
  Dx12SynchronizationWaitResult wait(HANDLE) noexcept override { return {WAIT_FAILED, ERROR_ACCESS_DENIED}; }
  HRESULT removed(ID3D12Device& device) noexcept override { return native.removed(device); }
  void pause() noexcept override { native.pause(); }
};
struct RestoreNative { Dx12State& state; ~RestoreNative() { state.synchronization_api = nullptr; } };
}
int main() {
  HideCompletion hidden;
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "unknown completion fixture");
  RestoreNative restore{*state};
  glyph_add_record(*state, 1);
  auto snapshot = glyph_publish(*state);
  if (!snapshot) return dx12_fail(2, "unknown completion snapshot");
  auto draw = glyph_draw(snapshot);
  state->synchronization_api = &hidden;
  const auto failed = dx12_submit_frame(*state, {0, 0, 0, 1}, std::span{&draw, 1}, true, 1);
  auto& owner = *state->texture_renderer->glyph_native;
  const auto retained = owner.stats();
  if (failed || !state->submitted || !retained.executed_unknown || !retained.pins || !retained.resources ||
      retained.completed_families || state->texture_renderer->native_textures.at(snapshot.get()).uploaded ||
      state->srv_pool->stats().live_slots != 1 || state->pixels || state->diagnostics)
    return dx12_fail(3, "event failure promoted or released unknown real GPU work");
  const auto used = state->texture_uploads->stats().used_bytes;
  const auto second = dx12_submit_frame(*state, {1, 0, 0, 1}, {}, true, 0);
  if (second || !state->submitted || owner.stats().pins != retained.pins ||
      owner.stats().resources != retained.resources || state->texture_uploads->stats().used_bytes != used ||
      state->srv_pool->stats().live_slots != 1)
    return dx12_fail(4, "unproven followup rewound or freed native dependencies");
  state->synchronization_api = nullptr;
  if (!dx12_wait_idle(*state)) return dx12_fail(5, "real native ticket could not prove completion");
  dx12_commit_texture_uploads(*state);
  if (owner.stats().executed_unknown || owner.stats().completed_families != 1 ||
      owner.stats().resources != retained.resources || owner.stats().pins != retained.pins)
    return dx12_fail(6, "G must publish progress but preserve recorded dependencies until P");
  draw = {}; snapshot.reset(); state->text_cache->pages[0].texture.reset();
  dx12_begin_texture_frame(*state);
  if (owner.stats().detached_nodes != 1 || state->srv_pool->stats().live_slots != 1)
    return dx12_fail(7, "completed-before-P exact node lost its slot");
  if (!glyph_submit(*state, {}) || owner.stats().detached_nodes || owner.stats().resources ||
      state->srv_pool->stats().live_slots)
    return dx12_fail(8, "successful parent Reset failed to release recorded dependencies");
  return 0;
}
