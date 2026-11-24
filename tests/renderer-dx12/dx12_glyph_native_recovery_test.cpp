#include "dx12_glyph_native_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_recovery_internal.hpp"
#include <array>

using namespace cgpui;
using namespace cgpui_test;
int main() {
  Dx12RecoveryDeadline deadline;
  std::array<Dx12TextureCopyEvent, 128> events{};
  Dx12TextureCopyObserver observer(events);
  std::atomic_bool old_released = false;
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!deadline.done || !state) return dx12_fail(1, "glyph recovery fixture");
  state->texture_renderer->glyph_native->observer = &observer;
  glyph_add_record(*state, 1);
  auto a = glyph_publish(*state);
  if (!a) return dx12_fail(2, "recovery A publication");
  auto draw = glyph_draw(a);
  if (!glyph_submit(*state, std::span{&draw, 1}) || !glyph_pixels(*state, *a) ||
      !dx12_attach_resource_sentinel(*state->texture_renderer->native_textures.at(a.get()).resource.Get(), old_released))
    return dx12_fail(3, "recovery A native pixels/sentinel");
  const auto original_info = state->info;
  if (!dx12_remove_native_device(original_info)) return dx12_fail(4, "real glyph device removal");
  const auto recovered = dx12_recover_removed_device(*state);
  if (!recovered || !*recovered || !old_released.load() ||
      state->texture_renderer->glyph_native->stats().completed_families ||
      state->texture_renderer->glyph_native->stats().pins ||
      state->info.device_generation != original_info.device_generation + 1)
    return dx12_fail(5, "native epoch reset retained old glyph catalog/dependencies");
  observer.clear();
  if (!glyph_submit(*state, std::span{&draw, 1}) || !glyph_pixels(*state, *a) ||
      glyph_copy_bytes(observer, Dx12TextureCopyKind::full_upload) != a->pixels.size() ||
      glyph_copy_count(observer, Dx12TextureCopyKind::clone))
    return dx12_fail(6, "held CPU snapshot reused old native epoch");
  observer.clear();
  if (!glyph_submit(*state, std::span{&draw, 1}) || !observer.events().empty() ||
      !dx12_recovered_native_device_clean(state->info))
    return dx12_fail(7, "fresh epoch exact hit did not reuse native glyph");
  return 0;
}
