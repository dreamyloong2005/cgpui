#include "dx12_glyph_suffix_internal.hpp"
#include "dx12_glyph_record_validation_internal.hpp"
#include <limits>

namespace cgpui {
namespace {
Error invalid_suffix() {
  return {.code = ErrorCode::invalid_argument, .message = "DX12 glyph suffix does not match a sealed page history"};
}
}
Result<Dx12GlyphSuffix> dx12_glyph_suffix(
    const Dx12GlyphLineage& lineage, const Dx12GlyphSnapshot& target, std::size_t base,
    std::span<const GlyphUploadRecord> history, std::span<const GlyphAtlasPage> pages) {
  const auto indices = lineage.record_indices(target);
  if (!indices || base >= target.prefix) return std::unexpected(invalid_suffix());
  const GlyphAtlasPage* page = nullptr;
  for (const auto& candidate : pages) if (candidate.page_index == target.page_index) {
    if (page) return std::unexpected(invalid_suffix());
    page = &candidate;
  }
  if (!page) return std::unexpected(invalid_suffix()); // Never accept planner's missing-page fallback.
  const auto full = std::uint64_t(target.width) * target.height;
  if (!full || full > std::numeric_limits<std::size_t>::max()) return std::unexpected(invalid_suffix());
  Dx12GlyphSuffix result{.target = target, .base_prefix = base, .full_bytes = std::size_t(full)};
  std::size_t previous = 0;
  for (std::size_t k = 0; k < target.prefix; ++k) {
    const auto global = (*indices)[k];
    if (global >= history.size() || (k && global <= previous) ||
        history[global].page_index != target.page_index) return std::unexpected(invalid_suffix());
    previous = global;
    if (k < base) continue;
    auto layout = dx12_validate_glyph_record(history[global], *page, target.width, target.height);
    if (!layout) return std::unexpected(layout.error());
    const auto max = std::numeric_limits<std::size_t>::max();
    if (layout->tight_bytes > max - result.dirty_bytes ||
        history[global].alpha.size() > max - result.planner_bytes) return std::unexpected(invalid_suffix());
    result.dirty_bytes += layout->tight_bytes;
    result.planner_bytes += history[global].alpha.size();
  }
  // Every earlier same-page record must be represented once: no omitted page-local ordinal.
  std::size_t ordinal = 0;
  for (std::size_t global = 0; global <= indices->back(); ++global) {
    if (history[global].page_index != target.page_index) continue;
    if (ordinal >= target.prefix || (*indices)[ordinal] != global) return std::unexpected(invalid_suffix());
    ++ordinal;
  }
  if (ordinal != target.prefix) return std::unexpected(invalid_suffix());
  result.records.reserve(target.prefix - base);
  for (auto k = base; k < target.prefix; ++k) result.records.push_back({k + 1, (*indices)[k]});
  return result;
}
} // namespace cgpui
