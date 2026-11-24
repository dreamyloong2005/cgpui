#include "dx12_glyph_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_recording_scope_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_upload_frame_internal.hpp"
#include <array>
#include <cstdlib>
#include <new>

namespace {
thread_local std::size_t fail_at = 0, allocations = 0;
thread_local bool enabled = false;
void* allocate(std::size_t size) {
  if (enabled && ++allocations == fail_at) throw std::bad_alloc{};
  if (auto* value = std::malloc(size ? size : 1)) return value;
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
  std::array<Dx12TextureCopyEvent, 128> events{};
  Dx12TextureCopyObserver observer(events);
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "later-region fixture");
  auto& renderer = *state->texture_renderer;
  auto& owner = *renderer.glyph_native;
  owner.observer = &observer;
  // Each padded native region footprint exceeds256KiB, so second region needs another pool page.
  glyph_add_record(*state, 1, 3, 1100, 5);
  auto a = glyph_publish(*state);
  if (!a) return dx12_fail(2, "tall glyph A publication");
  auto draw = glyph_draw(a);
  if (!glyph_submit(*state, std::span{&draw, 1}) || !glyph_pixels(*state, *a))
    return dx12_fail(3, "tall glyph completed A");
  glyph_add_record(*state, 2, 3, 1100, 5); glyph_add_record(*state, 3, 3, 1100, 5);
  auto ab = glyph_publish(*state);
  if (!ab) return dx12_fail(4, "tall glyph AB publication");
  bool later_region = false;
  for (std::size_t point = 1; point <= 128 && !later_region; ++point) {
    if (!dx12_wait_idle(*state)) return dx12_fail(5, "later-region idle");
    dx12_begin_texture_frame(*state);
    observer.clear();
    bool threw = false;
    {
      Dx12GlyphRecordingScope scope(*state);
      if (FAILED(state->allocator->Reset()) || FAILED(state->commands->Reset(state->allocator.Get(), nullptr)))
        return dx12_fail(6, "later-region native Reset");
      scope.opened_after_reset();
      owner.begin_recording(state->text_cache->lineage);
      // Every previous attempt was never executed. P has now retired its references;
      // erase only its failed target to reproduce fresh target allocation, not production recovery.
      renderer.native_textures.erase(ab.get());
      if (!dx12_begin_texture_upload_frame(*state)) return dx12_fail(7, "later-region pool rewind");
      allocations = 0; fail_at = point; enabled = true;
      try { (void)dx12_prepare_texture(*state, ab, true); }
      catch (const std::bad_alloc&) { threw = true; }
      enabled = false;
      later_region = threw && state->texture_uploads->stats().used_bytes != 0;
      if (threw && (glyph_copy_count(observer, Dx12TextureCopyKind::clone) ||
                    glyph_copy_count(observer, Dx12TextureCopyKind::region_upload)))
        return dx12_fail(8, "fallible target preparation continued after its first command reference");
    }
    if (owner.stats().close_attempts != 1 || FAILED(owner.stats().last_close) || owner.stats().executed_unknown)
      return dx12_fail(9, "allocation exception did not close/abandon once");
    if (later_region && (!owner.stats().resources || !owner.stats().pins ||
                         owner.stats().candidates || state->submitted))
      return dx12_fail(10, "later-region failure lost staging/node owners or published target progress");
  }
  return later_region ? 0 : dx12_fail(11, "did not reach an actual later-region pool allocation failure");
}
