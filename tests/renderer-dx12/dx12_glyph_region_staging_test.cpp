#include "dx12_glyph_cpu_test_support.hpp"
#include "dx12_texture_upload_test_support.hpp"
#include "../../src/renderer/dx12/dx12_texture_region_upload_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  auto state = texture_upload_state();
  if (!state) return dx12_fail(1, "glyph region staging device");
  GlyphCpuHistory fixture;
  auto plan = dx12_plan_glyph_regions(fixture.lineage, fixture.sealed, 2, fixture.records, fixture.pages);
  if (!plan || plan->chunks.size() != 2) return dx12_fail(2, "glyph region plan");
  UINT64 previous_end = 0;
  for (const auto& chunk : plan->chunks) {
    const auto& r = chunk.batch.uploads[0];
    auto slice = dx12_upload_glyph_region(*state, chunk);
    const Dx12Texture expected{.width = r.width, .height = r.height, .stride = r.stride,
        .format = DXGI_FORMAT_R8_UNORM, .pixels = chunk.batch.alpha};
    if (!slice || !staging_bytes(*slice, expected) || slice->footprint.Offset < previous_end ||
        slice->payload_bytes != std::size_t(r.width) * r.height)
      return dx12_fail(3, "odd-width/padded region footprint or resource-relative slice overlap");
    previous_end = slice->footprint.Offset + std::size_t(slice->footprint.Footprint.RowPitch) * (r.height - 1) + r.width;
  }
  auto invalid = plan->chunks.back();
  invalid.batch.alpha.resize(20);
  if (dx12_upload_glyph_region(*state, invalid)) return dx12_fail(4, "short final source row accepted");
  state->submitted = true; // Synthetic guard control only; no Execute occurred in this staging test.
  const auto before = texture_upload_stats(*state->texture_uploads);
  const auto denied = dx12_upload_glyph_region(*state, plan->chunks.front());
  state->submitted = false;
  if (denied || texture_upload_stats(*state->texture_uploads) != before)
    return dx12_fail(5, "unknown-submission guard mutated staging");
  return 0;
}
