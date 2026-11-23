#include "dx12_geometry_upload_test_support.hpp"
#include <vector>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  auto state = geometry_state();
  if (!state) return dx12_fail(1, "geometry retention device unavailable");
  std::array<std::atomic_bool, 7> released{}; // Outlives arena and every resource.
  Dx12GeometryUploads arena;
  std::array<ID3D12Resource*, 6> original{}; // Borrowed identity only, never AddRef.
  std::vector<unsigned char> bytes(geometry_page_bytes, 0x6d);
  for (std::size_t i = 0; i != original.size(); ++i) {
    auto slice = arena.upload(*state->device.Get(), bytes.data(), bytes.size());
    if (!slice || slice->offset || !geometry_bytes(*slice, bytes) ||
        !geometry_sentinel(*slice->resource, released[i]))
      return dx12_fail(2, "standard arena page/sentinel unavailable");
    original[i] = slice->resource;
  }
  bytes.resize(geometry_page_bytes + 1, 0x9a);
  auto oversized = arena.upload(*state->device.Get(), bytes.data(), bytes.size());
  if (!oversized || oversized->offset || oversized->resource->GetDesc().Width != 320 * 1024 ||
      !geometry_bytes(*oversized, bytes) || !geometry_sentinel(*oversized->resource, released[6]) ||
      arena.stats().page_count != 7 || arena.stats().capacity_bytes != 6 * geometry_page_bytes + 320 * 1024 ||
      arena.stats().allocations_this_frame != 7)
    return dx12_fail(3, "oversized page was not a distinct rounded transient allocation");
  const auto total = arena.stats().total_allocations;
  arena.begin_frame(); // No GPU work: allocator-only completion is already proven.
  std::size_t retained = 0;
  for (std::size_t i = 0; i != original.size(); ++i) retained += !released[i].load();
  if (retained != 4 || !released[6].load() || arena.stats().page_count != 4 ||
      arena.stats().capacity_bytes != 4 * geometry_page_bytes || arena.stats().used_bytes ||
      arena.stats().allocations_this_frame || arena.stats().total_allocations != total)
    return dx12_fail(4, "frame rewind did not enforce bounded retention and release oversized pages");
  bytes.resize(geometry_page_bytes);
  std::array<ID3D12Resource*, 4> reused{};
  for (std::size_t i = 0; i != reused.size(); ++i) {
    auto slice = arena.upload(*state->device.Get(), bytes.data(), bytes.size());
    if (!slice || !geometry_bytes(*slice, bytes)) return dx12_fail(5, "retained page reuse failed");
    bool found = false;
    for (std::size_t j = 0; j != original.size(); ++j)
      found |= !released[j].load() && slice->resource == original[j];
    for (std::size_t j = 0; j != i; ++j) if (slice->resource == reused[j]) found = false;
    if (!found) return dx12_fail(6, "retained pages were replaced or aliased within the frame");
    reused[i] = slice->resource;
  }
  if (arena.stats().allocations_this_frame || arena.stats().total_allocations != total)
    return dx12_fail(7, "steady-state reuse created new native resources");
  arena.clear();
  for (const auto& value : released) if (!value.load()) return dx12_fail(8, "clear leaked an arena page");
  const auto empty = arena.stats();
  if (empty.page_count || empty.capacity_bytes || empty.used_bytes || empty.allocations_this_frame ||
      empty.total_allocations != total) return dx12_fail(9, "clear counters lost allocation history");
  return 0;
}
