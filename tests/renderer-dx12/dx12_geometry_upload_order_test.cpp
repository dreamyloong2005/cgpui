#include "dx12_geometry_upload_test_support.hpp"
#include <vector>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  auto state = geometry_state();
  if (!state) return dx12_fail(1, "geometry order device unavailable");
  const std::array<unsigned char, 17> small_bytes{1, 2, 3, 4, 5};
  const std::vector<unsigned char> large(geometry_page_bytes + 9, 0xc7);
  for (const bool small_first : {false, true}) {
    Dx12GeometryUploads arena;
    Dx12GeometryUploadSlice first;
    if (small_first) {
      auto uploaded = arena.upload(*state->device.Get(), small_bytes.data(), small_bytes.size());
      if (!uploaded) return dx12_fail(2, "initial small geometry upload failed");
      first = *uploaded;
    }
    auto oversized = arena.upload(*state->device.Get(), large.data(), large.size());
    auto following = arena.upload(*state->device.Get(), small_bytes.data(), small_bytes.size());
    if (!oversized || !following || oversized->resource == following->resource ||
        !geometry_bytes(*oversized, large) || !geometry_bytes(*following, small_bytes) ||
        (small_first && (first.resource != following->resource || following->offset != 32 ||
                         !geometry_bytes(first, small_bytes))) ||
        (!small_first && following->offset) || arena.stats().page_count != 2 ||
        arena.stats().allocations_this_frame != 2)
      return dx12_fail(3, "oversized allocation stole the standard-page cursor or aliased data");
    auto* retained = following->resource;
    arena.begin_frame(); // Never submitted: safe reset of this allocator-only fixture.
    auto next = arena.upload(*state->device.Get(), small_bytes.data(), small_bytes.size());
    if (!next || next->resource != retained || next->offset || !geometry_bytes(*next, small_bytes) ||
        arena.stats().page_count != 1 || arena.stats().allocations_this_frame)
      return dx12_fail(4, "mixed-page frame did not retain/reuse its standard page");
  }
  return 0;
}
