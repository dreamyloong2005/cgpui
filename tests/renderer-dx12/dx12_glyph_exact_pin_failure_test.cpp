#include "dx12_glyph_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_recording_scope_internal.hpp"
#include "../../src/renderer/dx12/dx12_texture_upload_frame_internal.hpp"
#include <array>
#include <cstdlib>
#include <new>

namespace {
thread_local bool fail_next = false;
void* allocate(std::size_t size) {
  if (fail_next) { fail_next = false; throw std::bad_alloc{}; }
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
  std::array<Dx12TextureCopyEvent, 32> events{};
  Dx12TextureCopyObserver observer(events);
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "exact pin fixture");
  glyph_add_record(*state, 1);
  auto a = glyph_publish(*state);
  if (!a) return dx12_fail(2, "exact pin snapshot");
  auto draw = glyph_draw(a);
  if (!glyph_submit(*state, std::span{&draw, 1}) || !glyph_pixels(*state, *a))
    return dx12_fail(3, "exact pin completed source");
  auto& renderer = *state->texture_renderer;
  auto& owner = *renderer.glyph_native;
  auto* resource = renderer.native_textures.at(a.get()).resource.Get();
  const auto slots = state->srv_pool->stats().live_slots;
  owner.observer = &observer;
  bool caught = false;
  dx12_begin_texture_frame(*state);
  {
    Dx12GlyphRecordingScope scope(*state);
    if (FAILED(state->allocator->Reset()) || FAILED(state->commands->Reset(state->allocator.Get(), nullptr)))
      return dx12_fail(4, "exact pin actual parent Reset");
    scope.opened_after_reset();
    owner.begin_recording(state->text_cache->lineage);
    if (!dx12_begin_texture_upload_frame(*state) || owner.stats().pins)
      return dx12_fail(5, "exact pin fresh recording precondition");
    renderer.retained.push_back(a); // Exclude unrelated CPU-retention allocation from this seam.
    // Completed native map entry exists; first remaining allocation is the new exact-hit Pin.
    fail_next = true;
    try { (void)dx12_prepare_texture(*state, a, true); }
    catch (const std::bad_alloc&) { caught = true; }
    fail_next = false;
  }
  if (!caught || owner.stats().pins || owner.stats().resources || owner.stats().candidates ||
      owner.stats().close_attempts != 1 || FAILED(owner.stats().last_close) ||
      owner.stats().executed_unknown || !observer.events().empty() ||
      !renderer.pending_uploads.empty() || !renderer.native_textures.at(a.get()).uploaded ||
      renderer.native_textures.at(a.get()).resource.Get() != resource ||
      state->srv_pool->stats().live_slots != slots || state->texture_uploads->stats().used_bytes)
    return dx12_fail(6, "exact-hit pin failure crossed descriptor/reference/progress boundary");
  // No failed-list Execute or fake recovery: ordinary destruction safely releases the never-executed list.
  if (!dx12_check_debug_errors(*state)) return dx12_fail(7, "exact-hit raw Reset/Close debug gate");
  return 0;
}
