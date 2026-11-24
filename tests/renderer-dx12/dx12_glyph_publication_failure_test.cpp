#include "dx12_glyph_cpu_test_support.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"
#include "../../src/renderer/dx12/dx12_text_cache_internal.hpp"
#include <cstdlib>
#include <new>

namespace {
thread_local bool count_allocations = false;
thread_local std::size_t allocation_count = 0, fail_at = 0;
void* allocate(std::size_t size) {
  if (count_allocations && ++allocation_count == fail_at) throw std::bad_alloc{};
  if (void* value = std::malloc(size ? size : 1)) return value;
  throw std::bad_alloc{};
}
}
void* operator new(std::size_t size) { return allocate(size); }
void* operator new[](std::size_t size) { return allocate(size); }
void operator delete(void* value) noexcept { std::free(value); }
void operator delete[](void* value) noexcept { std::free(value); }
void operator delete(void* value, std::size_t) noexcept { std::free(value); }
void operator delete[](void* value, std::size_t) noexcept { std::free(value); }

using namespace cgpui;
using namespace cgpui_test;
namespace {
void add(Dx12TextCache& cache, unsigned id) {
  auto record = glyph_cpu_record(id, 0, 0, 0);
  (void)cache.glyphs.allocate({.key = record.key,
      .bitmap = {.width = record.width, .height = record.height,
                 .stride = record.stride, .alpha = record.alpha}});
}
std::size_t successor_allocation_count(Dx12State& state) {
  Dx12TextCache cache;
  add(cache, 1);
  glyph_require(bool(dx12_update_glyph_pages(state, cache)), "initial counting publish");
  add(cache, 2);
  allocation_count = 0; fail_at = 0; count_allocations = true;
  const auto result = dx12_update_glyph_pages(state, cache);
  count_allocations = false;
  glyph_require(bool(result), "counting successor publish");
  return allocation_count;
}
}
int main() {
  try {
    Dx12State state; // No native device or command creation.
    const auto count = successor_allocation_count(state);
    glyph_require(count >= 3, "fixture did not observe reserve/snapshot/seal allocations");
    bool reserved_failure = false, publication_failure = false;
    for (std::size_t fail = 1; fail <= count; ++fail) {
      Dx12TextCache cache;
      add(cache, 1);
      glyph_require(bool(dx12_update_glyph_pages(state, cache)), "initial failure publish");
      auto old = cache.pages[0].texture;
      const auto bytes = old->pixels;
      add(cache, 2);
      allocation_count = 0; fail_at = fail; count_allocations = true;
      bool threw = false;
      try { (void)dx12_update_glyph_pages(state, cache); }
      catch (const std::bad_alloc&) { threw = true; }
      count_allocations = false;
      glyph_require(threw && cache.pages[0].texture == old && old->pixels == bytes,
                    "allocation failure exposed replacement or mutated prior snapshot");
      glyph_require(cache.lineage.stats().record_indices == cache.processed_uploads &&
          cache.lineage.stats().weak_snapshots == 1, "failure left uncommitted mapping/sidecar");
      if (cache.processed_uploads == 1) reserved_failure = true;
      if (cache.processed_uploads == 2) {
        publication_failure = true;
        glyph_require(cache.pages[0].dirty, "failed snapshot/seal cleaned dirty page");
      }
      glyph_require(bool(dx12_update_glyph_pages(state, cache)) &&
          cache.processed_uploads == 2 && cache.lineage.stats().record_indices == 2 &&
          cache.lineage.find_snapshot(cache.pages[0].texture)->prefix == 2,
          "retry after allocation failure duplicated/lost upload or failed to seal");
    }
    glyph_require(reserved_failure && publication_failure, "required preparation/publication failure classes absent");
    return 0;
  } catch (const std::exception& error) {
    count_allocations = false;
    std::cerr << error.what() << '\n'; return 1;
  }
}
