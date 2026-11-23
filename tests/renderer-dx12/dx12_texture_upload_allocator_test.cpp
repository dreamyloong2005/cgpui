#include "dx12_texture_upload_test_support.hpp"
#include "../../src/renderer/dx12/dx12_upload_pages_internal.hpp"
#include <limits>

using namespace cgpui;
using namespace cgpui_test;

namespace {
auto stats(const Dx12UploadPages& pages) {
  const auto value = pages.stats();
  return std::make_tuple(value.page_count, value.capacity_bytes, value.used_bytes,
                        value.allocations_this_frame, value.total_allocations);
}
} // namespace

int main() {
  auto state = texture_upload_state();
  if (!state) return dx12_fail(1, "shared upload allocator device unavailable");
  Dx12UploadPages pages;
  auto a = pages.allocate(*state->device.Get(), 17, 16);
  auto b = pages.allocate(*state->device.Get(), 19, 512);
  auto c = pages.allocate(*state->device.Get(), 7, 16);
  if (!a || !b || !c || a->resource != b->resource || b->resource != c->resource ||
      a->offset != 0 || b->offset != 512 || c->offset != 544 ||
      b->mapped != a->mapped + 512 || c->mapped != a->mapped + 544 ||
      a->size != 17 || b->size != 19 || c->size != 7 || pages.stats().used_bytes != 551)
    return dx12_fail(2, "shared allocator lost independent alignment or slice-local mapping");
  std::memset(a->mapped, 0x29, a->size);
  std::memset(b->mapped, 0x71, b->size);
  std::memset(c->mapped, 0x53, c->size);
  const auto before = stats(pages);
  const std::array<std::pair<std::size_t, std::size_t>, 5> invalid{{
      {16, 0}, {16, 3}, {16, 128 * 1024}, {0, 16},
      {std::numeric_limits<std::size_t>::max(), 512}}};
  for (const auto [size, alignment] : invalid) {
    const auto result = pages.allocate(*state->device.Get(), size, alignment);
    if (result || result.error().code != ErrorCode::invalid_argument || stats(pages) != before)
      return dx12_fail(3, "invalid shared allocation mutated page ownership or accounting");
  }
  for (const auto [slice, expected] : {std::pair{*a, std::byte{0x29}},
                                      std::pair{*b, std::byte{0x71}},
                                      std::pair{*c, std::byte{0x53}}})
    for (std::size_t i = 0; i != slice.size; ++i)
      if (slice.mapped[i] != expected) return dx12_fail(4, "shared page slices overlapped or were overwritten");
  // No GPU commands were recorded: the direct owner can safely rewind here.
  pages.begin_frame();
  auto reused = pages.allocate(*state->device.Get(), 17, 16);
  if (!reused || reused->resource != a->resource || reused->offset ||
      reused->mapped != a->mapped || pages.stats().allocations_this_frame)
    return dx12_fail(5, "shared native page failed safe reset/reuse");
  return 0;
}
