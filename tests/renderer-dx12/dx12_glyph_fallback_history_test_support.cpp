#include "dx12_glyph_fallback_controls_test_support.hpp"
#include "../../src/renderer/dx12/dx12_glyph_record_validation_internal.hpp"
#include <cstring>
#include <memory>
#include <utility>

namespace cgpui_test {
GlyphFallbackHistory::GlyphFallbackHistory() {
  for (unsigned page = 0; page != 2; ++page) {
    glyph_require(bool(lineage.ensure_page(page, 16, 16)), "fallback family creation");
    alpha[page].resize(256);
  }
}
cgpui::Dx12TextureHandle GlyphFallbackHistory::append(
    unsigned page, unsigned id, unsigned x, unsigned y) {
  glyph_require(page < pages.size(), "fallback fixture page index");
  auto record = glyph_cpu_record(id, page, x, y, 3, 2, 5);
  pages[page].entries.push_back({.key = record.key, .page_index = page, .atlas_bounds = record.atlas_bounds});
  glyph_require(bool(cgpui::dx12_validate_glyph_record(record, pages[page], 16, 16)),
      "fallback source record must pass unchanged real validator");
  const auto index = records.size();
  glyph_require(bool(lineage.prepare_record(page, index)), "fallback mapping reserve");
  for (unsigned row = 0; row < record.height; ++row)
    std::memcpy(alpha[page].data() + std::size_t(y + row) * 16 + x,
        record.alpha.data() + std::size_t(row) * record.stride, record.width);
  records.push_back(std::move(record));
  lineage.commit_record(page, index);
  auto snapshot = std::make_shared<const cgpui::Dx12Texture>(cgpui::Dx12Texture{
      .width = 16, .height = 16, .stride = 16, .format = DXGI_FORMAT_R8_UNORM, .pixels = alpha[page]});
  glyph_require(bool(lineage.seal_snapshot(page, snapshot)), "fallback immutable snapshot seal");
  return snapshot;
}
cgpui::Dx12GlyphTargetInputs GlyphFallbackHistory::inputs() const { return {&lineage, records, pages}; }
bool glyph_fallback_full_copy(const cgpui::Dx12TextureCopyObserver& observer,
    ID3D12Resource& resource, const cgpui::Dx12Texture& texture) {
  using namespace cgpui;
  if (observer.overflowed() || observer.events().size() != 1 ||
      glyph_copy_count(observer, Dx12TextureCopyKind::clone) ||
      glyph_copy_count(observer, Dx12TextureCopyKind::region_upload)) return false;
  const auto& copy = observer.events()[0];
  return copy.kind == Dx12TextureCopyKind::full_upload && copy.target == &resource && copy.source &&
      copy.payload_bytes == texture.pixels.size() && copy.x == 0 && copy.y == 0 &&
      copy.width == texture.width && copy.height == texture.height && copy.row_pitch >= texture.width &&
      copy.row_pitch % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT == 0 &&
      copy.offset % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT == 0;
}
cgpui::Result<void> glyph_fallback_readback(cgpui::Dx12State& state,
    ID3D12Resource& resource, const cgpui::Dx12Texture& expected) {
  auto whole = glyph_readback(state, resource); // Requires validation revision's shared helper.
  if (!whole) return std::unexpected(whole.error());
  if (auto clean = cgpui::dx12_check_debug_errors(state); !clean) return clean;
  if (*whole != expected.pixels) return std::unexpected(cgpui::Error{
      .code = cgpui::ErrorCode::invalid_argument, .message = "private fallback full-page bytes differ"});
  return {};
}
} // namespace cgpui_test
