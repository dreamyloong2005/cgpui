#include "dx12_glyph_cpu_test_support.hpp"
#include <cstdlib>
#include <new>

namespace {
struct Probe { std::size_t calls = 0, bytes = 0; bool enabled = false, fail_next = false; };
thread_local Probe probe;
struct Measurement {
  explicit Measurement(bool fail = false) { probe = {0, 0, true, fail}; }
  ~Measurement() { probe.enabled = false; probe.fail_next = false; }
};
void* allocate(std::size_t size) {
  if (probe.enabled) {
    ++probe.calls; probe.bytes += size;
    if (probe.fail_next) { probe.fail_next = false; throw std::bad_alloc{}; }
  }
  if (void* value = std::malloc(size ? size : 1)) return value;
  throw std::bad_alloc{};
}
} // namespace
// Only this isolated CPU executable replaces ordinary allocation. No GPU/device or public stats.
void* operator new(std::size_t size) { return allocate(size); }
void* operator new[](std::size_t size) { return allocate(size); }
void operator delete(void* value) noexcept { std::free(value); }
void operator delete[](void* value) noexcept { std::free(value); }
void operator delete(void* value, std::size_t) noexcept { std::free(value); }
void operator delete[](void* value, std::size_t) noexcept { std::free(value); }

using namespace cgpui;
using namespace cgpui_test;
namespace {
constexpr std::size_t records = 4096;
constexpr std::size_t global_index(std::size_t index) { return index * 3 + 7; }
void check_indices(const Dx12GlyphLineage& lineage, const Dx12GlyphSnapshot& seal, std::size_t count) {
  const auto indices = lineage.record_indices(seal);
  glyph_require(indices && indices->size() == count, "sealed growth prefix mismatch");
  for (std::size_t index = 0; index < count; ++index)
    glyph_require((*indices)[index] == global_index(index), "growth corrupted global-index mapping");
}
void growth() {
  Dx12GlyphLineage lineage;
  glyph_require(bool(lineage.ensure_page(0, 16, 16)), "growth family");
  bool valid = true, commit_allocated = false;
  {
    Measurement measured;
    for (std::size_t index = 0; index < records; ++index) {
      if (!lineage.prepare_record(0, global_index(index))) { valid = false; break; }
      const auto before = probe.calls;
      lineage.commit_record(0, global_index(index));
      commit_allocated |= probe.calls != before;
    }
  }
  const auto measured = probe;
  // Setup, snapshot/seal, validation and output are outside the measured insertion loop.
  auto snapshot = glyph_cpu_snapshot();
  glyph_require(valid && !commit_allocated && lineage.stats().record_indices == records &&
      bool(lineage.seal_snapshot(0, snapshot)), "growth changed prepare/commit semantics");
  check_indices(lineage, *lineage.find_snapshot(snapshot), records);
  std::cout << "lineage-growth records=" << records << " ordinary_new_calls=" << measured.calls
            << " requested_bytes=" << measured.bytes << '\n';
  glyph_require(measured.calls > 0 && measured.bytes >= records * sizeof(std::size_t),
      "ordinary-new probe did not observe real index storage");
  // Loose logarithmic/linear budgets, not a specific doubling sequence or allocator address.
  glyph_require(measured.calls <= 64 && measured.bytes <= 4 * records * sizeof(std::size_t),
      "lineage index growth must not reserve exact size for every record");
}
void failure_retry() {
  Dx12GlyphLineage lineage;
  glyph_require(bool(lineage.ensure_page(0, 16, 16)), "failure family");
  constexpr std::size_t seed = 64;
  for (std::size_t index = 0; index < seed; ++index) {
    glyph_require(bool(lineage.prepare_record(0, global_index(index))), "failure seed preparation");
    lineage.commit_record(0, global_index(index));
  }
  auto held = glyph_cpu_snapshot();
  glyph_require(bool(lineage.seal_snapshot(0, held)), "failure held seal");
  const auto old = *lineage.find_snapshot(held);
  std::size_t cursor = seed;
  bool threw = false, valid = true, commit_allocated = false;
  {
    Measurement failing(true);
    // Consume any spare capacity; deny the next real growth, not a guessed ordinal/capacity.
    for (; cursor < records; ++cursor) {
      try {
        if (!lineage.prepare_record(0, global_index(cursor))) { valid = false; break; }
      } catch (const std::bad_alloc&) { threw = true; break; }
      const auto before = probe.calls;
      lineage.commit_record(0, global_index(cursor));
      commit_allocated |= probe.calls != before;
    }
  }
  glyph_require(valid && threw && probe.calls == 1 && probe.bytes > 0 && !commit_allocated &&
      lineage.stats().record_indices == cursor && lineage.stats().weak_snapshots == 1 &&
      lineage.find_snapshot(held)->prefix == seed && lineage.current(old),
      "failed reserve changed prefix, seal or committed progress");
  check_indices(lineage, old, seed);
  {
    Measurement retry;
    valid = bool(lineage.prepare_record(0, global_index(cursor)));
    if (valid) {
      const auto before = probe.calls;
      lineage.commit_record(0, global_index(cursor));
      commit_allocated |= probe.calls != before;
    }
  }
  glyph_require(valid && probe.calls > 0 && probe.bytes > 0 && !commit_allocated &&
      lineage.stats().record_indices == cursor + 1, "reserve retry lost or duplicated progress");
  glyph_require(!lineage.prepare_record(0, global_index(cursor)), "retry accepted duplicate global index");
  auto after = glyph_cpu_snapshot();
  glyph_require(bool(lineage.seal_snapshot(0, after)), "retry final seal");
  check_indices(lineage, *lineage.find_snapshot(after), cursor + 1);
  check_indices(lineage, old, seed);
  std::cout << "lineage-growth allocation failure preserved prefix=" << cursor << "; retry committed once\n";
}
} // namespace
int main() {
  try {
    failure_retry(); // Check exception safety even when the subsequent budget intentionally fails RED.
    growth();
    return 0;
  } catch (const std::exception& error) {
    probe.enabled = false; probe.fail_next = false;
    std::cerr << error.what() << '\n'; return 1;
  }
}
