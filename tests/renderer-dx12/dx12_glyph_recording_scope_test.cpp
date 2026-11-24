#include "dx12_glyph_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_recording_scope_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_upload_frame_internal.hpp"
#include <new>

using namespace cgpui;
using namespace cgpui_test;
int main() {
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "scope fixture");
  glyph_add_record(*state, 1);
  auto snapshot = glyph_publish(*state);
  if (!snapshot || !dx12_wait_idle(*state)) return dx12_fail(2, "scope snapshot");
  auto& owner = *state->texture_renderer->glyph_native;
  try {
    Dx12GlyphRecordingScope scope(*state);
    if (!scope.entered()) return dx12_fail(3, "outer scope");
    Dx12GlyphRecordingScope nested(*state);
    if (nested.entered()) return dx12_fail(4, "nested scope entered");
    if (FAILED(state->allocator->Reset()) || FAILED(state->commands->Reset(state->allocator.Get(), nullptr)))
      return dx12_fail(5, "scope native Reset");
    scope.opened_after_reset();
    owner.begin_recording(state->text_cache->lineage);
    if (!dx12_begin_texture_upload_frame(*state)) return dx12_fail(6, "scope staging");
    if (!dx12_prepare_texture(*state, snapshot, true)) return dx12_fail(7, "scope native target");
    throw std::bad_alloc{}; // A later draw's allocation failure after the target has recorded references.
  } catch (const std::bad_alloc&) {}
  auto stats = owner.stats();
  if (stats.scope_active || stats.close_attempts != 1 || FAILED(stats.last_close) ||
      stats.executed_unknown || !stats.pins || !stats.resources ||
      state->texture_renderer->native_textures.at(snapshot.get()).uploaded)
    return dx12_fail(8, "exception did not Close once/preserve unexecuted dependencies");
  const auto slots = state->srv_pool->stats().live_slots;
  snapshot.reset();
  state->text_cache->pages[0].texture.reset();
  dx12_begin_texture_frame(*state);
  if (owner.stats().detached_nodes != 1 || state->srv_pool->stats().live_slots != slots)
    return dx12_fail(9, "unexecuted dependency freed before parent Reset");
  {
    Dx12GlyphRecordingScope scope(*state);
    if (FAILED(state->allocator->Reset()) || FAILED(state->commands->Reset(state->allocator.Get(), nullptr)))
      return dx12_fail(10, "safe followup Reset");
    scope.opened_after_reset();
    owner.begin_recording(state->text_cache->lineage);
    if (owner.stats().detached_nodes || state->srv_pool->stats().live_slots)
      return dx12_fail(11, "Reset did not retire unexecuted dependencies");
    if (FAILED(scope.close())) return dx12_fail(12, "explicit Close");
  }
  if (owner.stats().close_attempts != 1) return dx12_fail(13, "scope attempted Close twice");
  return 0;
}
