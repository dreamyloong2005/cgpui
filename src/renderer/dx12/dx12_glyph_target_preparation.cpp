#include "dx12_glyph_native_recording_internal.hpp"
#include "dx12_texture_renderer_internal.hpp"
#include "dx12_text_cache_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_texture_region_upload_internal.hpp"
#include "dx12_texture_upload_frame_internal.hpp"
#include "dx12_glyph_work_observer_internal.hpp"
#include <limits>

namespace cgpui {
Result<Dx12GlyphPreparedTarget> dx12_prepare_glyph_target(
    Dx12State& state, const Dx12TextureHandle& texture, Dx12NativeTexture& native) {
  const auto* cache = state.text_cache.get();
  return dx12_prepare_glyph_target(state, texture, native, cache
      ? Dx12GlyphTargetInputs{&cache->lineage, cache->glyphs.upload_records(), cache->glyphs.atlas_pages()}
      : Dx12GlyphTargetInputs{});
}
Result<Dx12GlyphPreparedTarget> dx12_prepare_glyph_target(
    Dx12State& state, const Dx12TextureHandle& texture, Dx12NativeTexture& native,
    const Dx12GlyphTargetInputs& inputs) {
  auto& owner = *state.texture_renderer->glyph_native;
  Dx12GlyphPreparedTarget result;
  if (inputs.lineage) result.snapshot = inputs.lineage->find_snapshot(texture);
  if (result.snapshot) {
    if (texture->format != DXGI_FORMAT_R8_UNORM || texture->width != result.snapshot->width ||
        texture->height != result.snapshot->height || !inputs.lineage->current(*result.snapshot))
      return std::unexpected(Error{.code = ErrorCode::invalid_argument,
          .message = "DX12 glyph snapshot no longer matches its sealed family"});
    result.base = owner.completed_base(*result.snapshot);
    if (result.base.resource) {
      auto plan = dx12_plan_glyph_regions(*inputs.lineage, *result.snapshot, result.base.prefix,
          inputs.history, inputs.pages);
      if (!plan) return std::unexpected(plan.error()); // Invalid CPU source never becomes full fallback.
      if (plan->dirty_bytes && plan->dirty_bytes < plan->full_bytes) {
        result.regions = std::move(*plan);
      } else {
        dx12_observe_glyph_plan(owner.work_observer, *plan);
        result.base = {}; // Valid D>=P uses full initialization, never a wider COW copy claim.
      }
    }
    owner.prepare_candidate(*result.snapshot); // Actual map node and candidate storage.
  }
  const auto copies = result.regions ? result.regions->chunks.size() : 1U;
  if (copies > std::numeric_limits<std::size_t>::max() - 2)
    return std::unexpected(Error{.code = ErrorCode::invalid_argument, .message = "DX12 glyph region count overflow"});
  const auto observations = copies + (result.regions ? 1U : 0U) + (!native.resource ? 1U : 0U);
  if (owner.observer && !owner.observer->capacity_for(observations))
    return std::unexpected(Error{.code = ErrorCode::invalid_argument, .message = "DX12 glyph observation capacity exhausted"});
  owner.pin(native);
  owner.reserve_resources(copies + (result.regions ? 1U : 0U));
  result.staging.reserve(copies);
  return result;
}
Result<void> dx12_stage_glyph_target(
    Dx12State& state, const Dx12Texture& texture, Dx12GlyphPreparedTarget& prepared) {
  auto& owner = *state.texture_renderer->glyph_native;
  if (prepared.regions) {
    dx12_observe_glyph_plan(owner.work_observer, *prepared.regions); // Final post-return/move buffers.
    owner.retain(*prepared.base.resource.Get()); // Source pin before any clone/barrier reference.
    for (const auto& chunk : prepared.regions->chunks) {
      auto slice = dx12_upload_glyph_region(state, chunk);
      if (!slice) return std::unexpected(slice.error());
      owner.retain(*slice->resource);
      prepared.staging.push_back(*slice); // Storage was prepared before any commands.
      dx12_observe_glyph_lifetime(state);
    }
  } else {
    auto slice = dx12_upload_texture(state, texture);
    if (!slice) return std::unexpected(slice.error());
    owner.retain(*slice->resource);
    prepared.staging.push_back(*slice);
    dx12_observe_glyph_lifetime(state);
  }
  return {};
}
} // namespace cgpui
