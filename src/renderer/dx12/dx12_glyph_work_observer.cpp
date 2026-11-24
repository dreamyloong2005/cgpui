#include "dx12_glyph_work_observer_internal.hpp"
#include "dx12_glyph_upload_plan_internal.hpp"
#include "dx12_glyph_native_internal.hpp"
#include "dx12_texture_renderer_internal.hpp"
#include "dx12_texture_upload_internal.hpp"
#include "dx12_srv_descriptor_pool_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
Dx12GlyphWorkObserver::Dx12GlyphWorkObserver(
    std::span<Dx12GlyphWorkEvent> events, std::span<ID3D12Resource*> textures) noexcept
    : events_(events), textures_(textures) {}
void Dx12GlyphWorkObserver::record(Dx12GlyphWorkEvent event) noexcept {
  if (used_ == events_.size()) { overflowed_ = true; return; }
  events_[used_++] = event;
}
void Dx12GlyphWorkObserver::begin_texture_sample() noexcept { texture_count_ = 0; }
void Dx12GlyphWorkObserver::include_texture(ID3D12Resource* resource) noexcept {
  if (!resource || resource->GetDesc().Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D) return;
  for (std::size_t i = 0; i < texture_count_; ++i) if (textures_[i] == resource) return;
  if (texture_count_ == textures_.size()) { overflowed_ = true; return; }
  textures_[texture_count_++] = resource;
}
std::size_t Dx12GlyphWorkObserver::texture_count() const noexcept { return texture_count_; }
bool Dx12GlyphWorkObserver::overflowed() const noexcept { return overflowed_; }
std::span<const Dx12GlyphWorkEvent> Dx12GlyphWorkObserver::events() const noexcept { return events_.first(used_); }
void Dx12GlyphWorkObserver::clear() noexcept { used_ = 0; texture_count_ = 0; overflowed_ = false; }
void dx12_observe_glyph_plan(Dx12GlyphWorkObserver* observer, const Dx12GlyphUploadPlan& plan) noexcept {
  if (!observer) return;
  for (const auto& chunk : plan.chunks)
    observer->record({.kind = Dx12GlyphWorkKind::planner_chunk,
        .source_data = chunk.source_alpha_data, .planned_data = chunk.planner_alpha_data,
        .handed_off_data = chunk.batch.alpha.data(), .payload = chunk.batch.alpha.size(),
        .capacity = chunk.batch.alpha.capacity(), .source_index = chunk.source.global_index});
}
void dx12_observe_glyph_lifetime(Dx12State& state) noexcept {
  auto& renderer = *state.texture_renderer;
  auto* observer = renderer.glyph_native->work_observer;
  if (!observer) return;
  observer->begin_texture_sample();
  for (const auto& [key, native] : renderer.native_textures) observer->include_texture(native.resource.Get());
  renderer.glyph_native->observe_owned_textures(*observer);
  const auto owned = renderer.glyph_native->stats();
  const auto pool = state.texture_uploads->stats();
  observer->record({.kind = Dx12GlyphWorkKind::owned_resources,
      .texture_count = observer->texture_count(), .live_slots = state.srv_pool->stats().live_slots,
      .dependency_refs = owned.resources, .catalog_count = owned.completed_families,
      .pool_pages = pool.page_count, .pool_capacity = pool.capacity_bytes, .pool_used = pool.used_bytes});
}
} // namespace cgpui
