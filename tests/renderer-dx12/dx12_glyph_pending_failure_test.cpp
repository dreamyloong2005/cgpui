#include "dx12_glyph_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_recording_scope_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_upload_frame_internal.hpp"
#include <cstdlib>
#include <new>

namespace {
thread_local bool fail_next = false;
void* allocate(std::size_t size) {
  if (fail_next) { fail_next = false; throw std::bad_alloc{}; }
  if (auto* result = std::malloc(size ? size : 1)) return result;
  throw std::bad_alloc{};
}
}
void* operator new(std::size_t size) { return allocate(size); }
void* operator new[](std::size_t size) { return allocate(size); }
void operator delete(void* value) noexcept { std::free(value); }
void operator delete[](void* value) noexcept { std::free(value); }
void operator delete(void* value, std::size_t) noexcept { std::free(value); }
void operator delete[](void* value, std::size_t) noexcept { std::free(value); }

using namespace cgpui;
using namespace cgpui_test;
int main() {
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "pending marker fixture");
  glyph_add_record(*state, 1);
  auto snapshot = glyph_publish(*state);
  if (!snapshot) return dx12_fail(2, "pending marker snapshot");
  auto& renderer = *state->texture_renderer;
  renderer.retained.push_back(snapshot);
  auto [it, added] = renderer.native_textures.try_emplace(snapshot.get());
  it->second.source = snapshot; // Existing CPU-retention/map insertion is excluded from the injected seam.
  if (!renderer.pending_uploads.empty() || renderer.pending_uploads.capacity()) return dx12_fail(3, "marker capacity precondition");
  auto& owner = *renderer.glyph_native;
  bool caught = false;
  {
    Dx12GlyphRecordingScope scope(*state);
    if (FAILED(state->allocator->Reset()) || FAILED(state->commands->Reset(state->allocator.Get(), nullptr)))
      return dx12_fail(4, "pending native Reset");
    scope.opened_after_reset(); owner.begin_recording(state->text_cache->lineage);
    if (!dx12_begin_texture_upload_frame(*state)) return dx12_fail(5, "pending staging");
    fail_next = true;
    try { (void)dx12_prepare_texture(*state, snapshot, true); }
    catch (const std::bad_alloc&) { caught = true; }
    fail_next = false;
  }
  if (!caught || owner.stats().close_attempts != 1 || FAILED(owner.stats().last_close) ||
      !renderer.pending_uploads.empty() || it->second.resource || it->second.uploaded ||
      state->texture_uploads->stats().used_bytes || owner.stats().pins || owner.stats().completed_families)
    return dx12_fail(6, "pending marker reserve failure occurred after native reference/progress");
  return 0;
}
