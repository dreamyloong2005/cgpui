#include "dx12_texture_renderer_internal.hpp"
#include "dx12_glyph_native_internal.hpp"
#include "dx12_glyph_work_observer_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
void dx12_begin_texture_frame(Dx12State& state) {
  auto& renderer = *state.texture_renderer;
  renderer.pending_uploads.clear();
  renderer.retained.clear();
  renderer.glyph_native->prune_expired(renderer);
  dx12_observe_glyph_lifetime(state);
}
void dx12_commit_texture_uploads(Dx12State& state) {
  for (auto* native : state.texture_renderer->pending_uploads) native->uploaded = true;
  state.texture_renderer->pending_uploads.clear();
  state.texture_renderer->glyph_native->commit_completed();
  dx12_observe_glyph_lifetime(state);
}
} // namespace cgpui
