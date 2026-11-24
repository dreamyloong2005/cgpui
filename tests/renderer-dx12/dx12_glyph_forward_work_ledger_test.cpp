#include "dx12_glyph_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_work_observer_internal.hpp"
#include <algorithm>
#include <array>
#include <iostream>

using namespace cgpui;
using namespace cgpui_test;
namespace {
int run(bool hold_a) {
  std::array<Dx12TextureCopyEvent, 128> copies{};
  std::array<Dx12GlyphWorkEvent, 256> work{};
  std::array<ID3D12Resource*, 128> scratch{};
  Dx12TextureCopyObserver copy_observer(copies);
  Dx12GlyphWorkObserver work_observer(work, scratch);
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "forward work fixture");
  auto& renderer = *state->texture_renderer;
  renderer.glyph_native->observer = &copy_observer;
  renderer.glyph_native->work_observer = &work_observer;
  glyph_add_record(*state, 1);
  auto a = glyph_publish(*state);
  if (!a) return dx12_fail(2, "forward A");
  auto first = glyph_draw(a);
  if (!glyph_submit(*state, std::span{&first, 1}) || !glyph_pixels(*state, *a))
    return dx12_fail(3, "forward completed A");
  const auto p = a->pixels.size();
  const auto old_bytes = a->pixels;
  auto* base = renderer.native_textures.at(a.get()).resource.Get();
  glyph_add_record(*state, 2);
  auto ab = glyph_publish(*state);
  glyph_add_record(*state, 3);
  auto abc = glyph_publish(*state);
  if (!ab || !abc) return dx12_fail(4, "forward successors");
  first = {};
  if (!hold_a) a.reset();
  std::array draws{glyph_draw(ab, 0, 0), glyph_draw(abc, 20, 1), glyph_draw(abc, 40, 2)};
  copy_observer.clear(); work_observer.clear();
  if (!glyph_submit(*state, draws) || !glyph_pixels(*state, *ab) ||
      !glyph_pixels(*state, *abc, 20) || !glyph_pixels(*state, *abc, 40))
    return dx12_fail(5, "forward/duplicate exact draw pixels");
  for (const auto& snapshot : {ab, abc}) {
    auto read = glyph_readback(*state, *renderer.native_textures.at(snapshot.get()).resource.Get());
    if (!read || *read != snapshot->pixels) return dx12_fail(6, "forward whole target page");
  }
  if (hold_a) {
    auto old = glyph_readback(*state, *base);
    if (!old || *old != old_bytes || a->pixels != old_bytes) return dx12_fail(7, "forward held source changed");
  }
  const auto history = state->text_cache->glyphs.upload_records();
  const auto tight = [](const GlyphUploadRecord& r) { return std::size_t(r.width) * r.height; };
  const auto expected_d = 2 * tight(history[1]) + tight(history[2]);
  const auto expected_q = 2 * history[1].alpha.size() + history[2].alpha.size();
  std::size_t q = 0, relocated = 0, chunks = 0, capacity = 0, barriers = 0;
  std::size_t peak_textures = 0, peak_slots = 0, peak_refs = 0, peak_pool_pages = 0, peak_pool_capacity = 0;
  for (const auto& event : work_observer.events()) {
    if (event.kind == Dx12GlyphWorkKind::planner_chunk) {
      if (!event.source_data || !event.planned_data || event.source_data == event.planned_data)
        return dx12_fail(8, "planner copy witness missing or source buffer aliased");
      q += event.payload; capacity += event.capacity; ++chunks;
      if (event.planned_data != event.handed_off_data) relocated += event.payload;
    } else if (event.kind == Dx12GlyphWorkKind::barrier) {
      if (!event.resource || !((event.before == D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE &&
              event.after == D3D12_RESOURCE_STATE_COPY_SOURCE) ||
          (event.before == D3D12_RESOURCE_STATE_COPY_SOURCE && event.after == D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) ||
          (event.before == D3D12_RESOURCE_STATE_COPY_DEST && event.after == D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)))
        return dx12_fail(9, "unexpected observed glyph barrier");
      ++barriers;
    } else {
      peak_textures = std::max(peak_textures, event.texture_count);
      peak_slots = std::max(peak_slots, event.live_slots);
      peak_refs = std::max(peak_refs, event.dependency_refs);
      peak_pool_pages = std::max(peak_pool_pages, event.pool_pages);
      peak_pool_capacity = std::max(peak_pool_capacity, event.pool_capacity);
    }
  }
  if (q != expected_q || relocated || chunks != 3 || barriers != 6 ||
      peak_textures != 3 || peak_slots != (hold_a ? 3U : 2U) || peak_refs != 5 || !peak_pool_pages ||
      glyph_copy_count(copy_observer, Dx12TextureCopyKind::allocation) != 2 ||
      glyph_copy_count(copy_observer, Dx12TextureCopyKind::clone) != 2 ||
      glyph_copy_bytes(copy_observer, Dx12TextureCopyKind::clone) != 2 * p ||
      glyph_copy_count(copy_observer, Dx12TextureCopyKind::region_upload) != 3 ||
      glyph_copy_bytes(copy_observer, Dx12TextureCopyKind::region_upload) != expected_d ||
      glyph_copy_count(copy_observer, Dx12TextureCopyKind::full_upload) ||
      copy_observer.overflowed() || work_observer.overflowed())
    return dx12_fail(10, "forward independent suffix/duplicate/actual work ledger mismatch");
  for (const auto& event : copy_observer.events())
    if (event.kind == Dx12TextureCopyKind::clone && event.source != base)
      return dx12_fail(11, "forward pending AB seeded ABC instead of completed A");
  std::size_t h = 0;
  for (const auto& record : history) h += record.alpha.size();
  std::cout << "glyph-work cohort=" << (hold_a ? "held" : "released")
      << " P_each=" << p << " D_observed=" << glyph_copy_bytes(copy_observer, Dx12TextureCopyKind::region_upload)
      << " Q_owned_copy_witness=" << q << " R_handoff_pointer_relocation=" << relocated
      << " planner_capacity_sum=" << capacity << " H_current=" << h
      << " clone_bytes_observed=" << glyph_copy_bytes(copy_observer, Dx12TextureCopyKind::clone)
      << " barriers_recorded_observed=" << barriers << " scoped_owned_texture_peak=" << peak_textures
      << " live_slot_peak=" << peak_slots << " dependency_ref_peak=" << peak_refs
      << " pool_page_peak=" << peak_pool_pages << " pool_capacity_peak=" << peak_pool_capacity
      << " AB_snapshot_size=" << ab->pixels.size() << " AB_snapshot_capacity=" << ab->pixels.capacity()
      << " ABC_snapshot_size=" << abc->pixels.size() << " ABC_snapshot_capacity=" << abc->pixels.capacity() << '\n';
  for (const auto& event : copy_observer.events()) if (event.kind == Dx12TextureCopyKind::region_upload)
    std::cout << "glyph-region source=" << event.source << " target=" << event.target
        << " x=" << event.x << " y=" << event.y << " width=" << event.width << " height=" << event.height
        << " pitch=" << event.row_pitch << " offset=" << event.offset << " payload=" << event.payload_bytes << '\n';
  // Printed metrics derive from actual events/enumeration, not these expected test constants.
  // No claim of whole-renderer VRAM, CPU instruction counts, GPU timing or speedup.
  auto* highest = renderer.native_textures.at(abc.get()).resource.Get();
  glyph_add_record(*state, 4);
  auto abcd = glyph_publish(*state);
  if (!abcd) return dx12_fail(12, "forward next snapshot");
  auto next = glyph_draw(abcd);
  copy_observer.clear(); work_observer.clear();
  if (!glyph_submit(*state, std::span{&next, 1}) || !glyph_pixels(*state, *abcd) ||
      glyph_copy_bytes(copy_observer, Dx12TextureCopyKind::region_upload) != tight(state->text_cache->glyphs.upload_records()[3]))
    return dx12_fail(13, "forward maximum completed prefix");
  for (const auto& event : copy_observer.events())
    if (event.kind == Dx12TextureCopyKind::clone && event.source != highest)
      return dx12_fail(14, "next target did not use completed ABC");
  copy_observer.clear(); work_observer.clear();
  if (!glyph_submit(*state, std::span{&next, 1}) || !copy_observer.events().empty())
    return dx12_fail(15, "cached target transported payload");
  for (const auto& event : work_observer.events())
    if (event.kind != Dx12GlyphWorkKind::owned_resources)
      return dx12_fail(16, "cached target planned payload or recorded glyph barriers");
  return 0;
}
}
int main() { if (const auto held = run(true)) return held; return run(false); }
