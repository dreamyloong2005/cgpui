#include "dx12_texture_upload_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  auto state = texture_upload_state();
  if (!state) return dx12_fail(1, "texture retention native device unavailable");
  std::array<std::atomic_bool, 7> released{};
  Dx12TextureUploads uploads;
  std::array<ID3D12Resource*, 6> pages{};
  const auto standard = staging_texture(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM, 4);
  for (std::size_t i = 0; i != pages.size(); ++i) {
    auto slice = uploads.upload(*state->device.Get(), standard);
    if (!slice || slice->footprint.Offset || !staging_bytes(*slice, standard) ||
        !dx12_attach_resource_sentinel(*slice->resource, released[i]))
      return dx12_fail(2, "standard texture page/sentinel unavailable");
    pages[i] = slice->resource;
  }
  const auto oversized = staging_texture(257, 256, DXGI_FORMAT_R8G8B8A8_UNORM, 8, 31);
  auto large = uploads.upload(*state->device.Get(), oversized);
  if (!large || large->footprint.Offset || large->resource->GetDesc().Width != 320 * 1024 ||
      !staging_bytes(*large, oversized) || !dx12_attach_resource_sentinel(*large->resource, released[6]) ||
      uploads.stats().page_count != 7 || uploads.stats().allocations_this_frame != 7)
    return dx12_fail(3, "oversize footprint was not a distinct rounded transient page");
  const auto total = uploads.stats().total_allocations;
  uploads.begin_frame(); // Direct allocator test never submitted GPU commands.
  std::size_t retained = 0;
  for (std::size_t i = 0; i != pages.size(); ++i) retained += !released[i].load();
  if (retained != 4 || !released[6].load() || uploads.stats().page_count != 4 ||
      uploads.stats().capacity_bytes != 4 * texture_page_bytes || uploads.stats().used_bytes ||
      uploads.stats().allocations_this_frame || uploads.stats().total_allocations != total)
    return dx12_fail(4, "safe texture reset failed its bounded page-retention contract");
  std::array<ID3D12Resource*, 4> reused{};
  for (std::size_t i = 0; i != reused.size(); ++i) {
    auto slice = uploads.upload(*state->device.Get(), standard);
    if (!slice || !staging_bytes(*slice, standard)) return dx12_fail(5, "texture retained-page copy failed");
    bool found = false;
    for (std::size_t j = 0; j != pages.size(); ++j) found |= !released[j].load() && slice->resource == pages[j];
    for (std::size_t j = 0; j != i; ++j) if (reused[j] == slice->resource) found = false;
    if (!found) return dx12_fail(6, "texture retained resources were replaced or aliased");
    reused[i] = slice->resource;
  }
  if (uploads.stats().allocations_this_frame || uploads.stats().total_allocations != total)
    return dx12_fail(7, "texture steady-state reuse created native pages");
  uploads.clear();
  for (const auto& flag : released) if (!flag.load()) return dx12_fail(8, "texture pool leaked native pages");
  return uploads.stats().page_count == 0 && uploads.stats().total_allocations == total
      ? 0 : dx12_fail(9, "texture clear lost history or retained pages");
}
