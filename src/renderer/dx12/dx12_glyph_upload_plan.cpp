#include "dx12_glyph_upload_plan_internal.hpp"
#include "cgpui/renderer/planning/glyph_resources.hpp"
#include "cgpui/renderer/planning/glyph_uploads.hpp"
#include <utility>

namespace cgpui {
Result<Dx12GlyphUploadPlan> dx12_plan_glyph_regions(
    const Dx12GlyphLineage& lineage, const Dx12GlyphSnapshot& target, std::size_t base,
    std::span<const GlyphUploadRecord> history, std::span<const GlyphAtlasPage> pages) {
  auto suffix = dx12_glyph_suffix(lineage, target, base, history, pages);
  if (!suffix) return std::unexpected(suffix.error());
  Dx12GlyphUploadPlan result{.target = target, .base_prefix = base, .full_bytes = suffix->full_bytes,
      .dirty_bytes = suffix->dirty_bytes, .planner_bytes = suffix->planner_bytes};
  result.chunks.reserve(suffix->records.size());
  for (const auto& index : suffix->records) {
    // Borrow exactly one original record; never copy GlyphUploadRecord/history coverage.
    auto batches = plan_glyph_atlas_uploads(history.subspan(index.global_index, 1), pages);
    if (batches.size() != 1 || batches[0].uploads.size() != 1)
      return std::unexpected(Error{.code = ErrorCode::renderer_initialization_failed,
          .message = "DX12 glyph one-record planner returned unexpected topology"});
    auto& batch = batches[0];
    const auto& region = batch.uploads[0];
    const auto& source = history[index.global_index];
    // Fresh on EVERY iteration: uploaded_count=1 from another record is not page progress.
    GlyphAtlasProductionResourceState production_state;
    auto production = plan_glyph_atlas_production_resources(production_state, batches);
    const auto last_row_end = std::uint64_t(region.stride) * (region.height - 1) + region.width;
    if (batch.image.page_index != target.page_index || batch.image.size.width != float(target.width) ||
        batch.image.size.height != float(target.height) || batch.image.upload_count != 1 ||
        batch.image.format != GlyphAtlasImageFormat::alpha8_unorm ||
        region.key != source.key || region.page_index != target.page_index ||
        region.atlas_bounds.origin.x != source.atlas_bounds.origin.x ||
        region.atlas_bounds.origin.y != source.atlas_bounds.origin.y ||
        region.atlas_bounds.size.width != source.atlas_bounds.size.width ||
        region.atlas_bounds.size.height != source.atlas_bounds.size.height || region.width != source.width ||
        region.height != source.height || region.stride != source.stride || region.byte_offset != 0 ||
        region.byte_size != source.alpha.size() || batch.alpha.size() != source.alpha.size() ||
        last_row_end > batch.alpha.size() || production.upload_commands.size() != 1 ||
        production.upload_commands[0].page_index != target.page_index ||
        production.upload_commands[0].first_upload_index != 0 ||
        production.upload_commands[0].upload_count != 1 ||
        production.upload_commands[0].byte_offset != 0 ||
        production.upload_commands[0].byte_size != batch.alpha.size() ||
        !production.upload_commands[0].stages_alpha_payload ||
        !production.upload_commands[0].transitions_to_transfer_dst ||
        !production.upload_commands[0].copies_buffer_to_image ||
        !production.upload_commands[0].transitions_to_shader_read_only)
      return std::unexpected(Error{.code = ErrorCode::renderer_initialization_failed,
          .message = "DX12 glyph one-record production adapter lost a sealed region"});
    const auto* planned_alpha = batch.alpha.data();
    result.chunks.push_back({index, std::move(batch), source.alpha.data(), planned_alpha});
  }
  return result;
}
} // namespace cgpui
