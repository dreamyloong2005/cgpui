#include "dx12_geometry_upload_test_support.hpp"
#include <limits>
#include <vector>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  auto state = geometry_state();
  if (!state) return dx12_fail(1, "geometry allocation device unavailable");
  Dx12GeometryUploads arena;
  const std::array<unsigned char, 17> first{1, 3, 5, 7, 11, 13, 17};
  const std::array<unsigned char, 19> second{2, 4, 6, 8, 10, 12, 14};
  auto a = arena.upload(*state->device.Get(), first.data(), first.size());
  auto b = arena.upload(*state->device.Get(), second.data(), second.size());
  if (!a || !b || a->resource != b->resource || a->offset != 0 || b->offset != 32 ||
      a->address + a->size > b->address || !geometry_bytes(*a, first) || !geometry_bytes(*b, second))
    return dx12_fail(2, "geometry slices alias, misalign, or copy bytes incorrectly");
  const auto before = geometry_stats(arena);
  if (arena.stats().page_count != 1 || arena.stats().capacity_bytes != geometry_page_bytes ||
      arena.stats().used_bytes != 51 || arena.stats().allocations_this_frame != 1)
    return dx12_fail(3, "geometry initial page/byte/allocation accounting differs");
  for (const auto size : {std::size_t{0}, std::size_t{UINT_MAX} + 1,
                          std::numeric_limits<std::size_t>::max()}) {
    const auto invalid = arena.upload(*state->device.Get(), first.data(), size);
    if (invalid || invalid.error().code != ErrorCode::invalid_argument || geometry_stats(arena) != before)
      return dx12_fail(4, "invalid/overflow geometry allocation mutated the arena");
  }
  const auto null = arena.upload(*state->device.Get(), nullptr, 16);
  if (null || null.error().code != ErrorCode::invalid_argument || geometry_stats(arena) != before ||
      !geometry_bytes(*a, first) || !geometry_bytes(*b, second))
    return dx12_fail(5, "null upload changed existing allocations or contents");
  arena.begin_frame(); // No command lists in this allocator-only test were submitted.
  std::vector<unsigned char> almost_full(geometry_page_bytes - 15, 0xa7);
  const std::array<unsigned char, 16> tail{0xe1, 0xe2, 0xe3, 0xe4};
  auto c = arena.upload(*state->device.Get(), almost_full.data(), almost_full.size());
  auto d = arena.upload(*state->device.Get(), tail.data(), tail.size());
  if (!c || !d || c->resource == d->resource || c->offset || d->offset ||
      arena.stats().page_count != 2 || arena.stats().allocations_this_frame != 1 ||
      !geometry_bytes(*c, almost_full) || !geometry_bytes(*d, tail))
    return dx12_fail(6, "cross-page aligned allocation overwrote or failed to reuse storage");
  return dx12_check_debug_errors(*state) ? 0 : dx12_fail(7, "geometry allocator emitted native debug errors");
}
