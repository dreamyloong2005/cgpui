#include "dx12_texture_upload_test_support.hpp"
#include <limits>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  auto state = texture_upload_state();
  if (!state) return dx12_fail(1, "texture layout native device unavailable");
  Dx12TextureUploads uploads;
  const auto rgba = staging_texture(3, 2, DXGI_FORMAT_R8G8B8A8_UNORM, 4, 7);
  const auto alpha = staging_texture(5, 3, DXGI_FORMAT_R8_UNORM, 4, 19);
  const auto wide = staging_texture(65, 2, DXGI_FORMAT_R8G8B8A8_UNORM, 12, 29);
  const auto a = uploads.upload(*state->device.Get(), rgba);
  const auto b = uploads.upload(*state->device.Get(), alpha);
  const auto c = uploads.upload(*state->device.Get(), wide);
  if (!a || !b || !c || a->resource != b->resource || b->resource != c->resource ||
      a->footprint.Offset != 0 || b->footprint.Offset != 512 || c->footprint.Offset != 1536 ||
      a->footprint.Footprint.RowPitch != 256 || b->footprint.Footprint.RowPitch != 256 ||
      c->footprint.Footprint.RowPitch != 512 || !staging_bytes(*a, rgba) ||
      !staging_bytes(*b, alpha) || !staging_bytes(*c, wide))
    return dx12_fail(2, "RGBA/R8 padded source rows or aligned nonzero footprint offsets differ");
  const auto before = texture_upload_stats(uploads);
  if (uploads.stats().page_count != 1 || uploads.stats().used_bytes != 2308 ||
      uploads.stats().capacity_bytes != texture_page_bytes || uploads.stats().allocations_this_frame != 1)
    return dx12_fail(3, "texture layout accounted last-row padding or lost slice alignment");
  for (unsigned fault = 0; fault != 6; ++fault) {
    auto invalid = rgba;
    if (fault == 0) invalid.width = 0;
    if (fault == 1) invalid.height = 0;
    if (fault == 2) invalid.width = std::numeric_limits<UINT>::max();
    if (fault == 3) invalid.format = DXGI_FORMAT_UNKNOWN;
    if (fault == 4) invalid.stride = 11;
    if (fault == 5) invalid.pixels.pop_back();
    const auto result = uploads.upload(*state->device.Get(), invalid);
    if (result || result.error().code != ErrorCode::invalid_argument || texture_upload_stats(uploads) != before)
      return dx12_fail(4, "invalid texture layout mutated allocation state");
  }
  return staging_bytes(*a, rgba) && staging_bytes(*b, alpha) && staging_bytes(*c, wide)
      ? 0 : dx12_fail(5, "failed texture upload overwrote prior footprint payloads");
}
