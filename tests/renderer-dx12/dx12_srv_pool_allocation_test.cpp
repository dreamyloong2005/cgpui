#include "dx12_srv_pool_test_support.hpp"
#include <set>
#include <type_traits>

using namespace cgpui;
using namespace cgpui_test;
static_assert(!std::is_copy_constructible_v<Dx12SrvAllocation>);
static_assert(std::is_nothrow_move_constructible_v<Dx12SrvAllocation>);
static_assert(std::is_nothrow_move_assignable_v<Dx12SrvAllocation>);
int main() {
  auto state = texture_upload_state();
  if (!state) return dx12_fail(1, "SRV allocation device unavailable");
  Dx12SrvDescriptorPool pool(4);
  auto a = pool.allocate(*state->device.Get(), 1), b = pool.allocate(*state->device.Get(), 1),
       c = pool.allocate(*state->device.Get(), 1);
  if (!a || !b || !c) return dx12_fail(2, "three image slots unavailable");
  auto* first = a->heap();
  const auto retained = c->gpu().ptr;
  const auto returned = b->gpu().ptr;
  *b = {}; // Return slot1, leaving noncontiguous free slots1/3 on first page.
  auto blur = pool.allocate(*state->device.Get(), 3);
  if (!blur || blur->heap() == first || blur->size() != 3 ||
      pool.stats().page_count != 2 || pool.stats().live_slots != 5 || c->gpu().ptr != retained)
    return dx12_fail(3, "blur3 split across fragmented pages or changed a live handle");
  // Two holes on page1 plus one on page2 are not a usable three-slot group.
  if (pool.stats().capacity - pool.stats().live_slots != 3)
    return dx12_fail(12, "fragmented global-free-count precondition failed");
  auto fragmented = pool.allocate(*state->device.Get(), 3);
  if (!fragmented || fragmented->heap() == first || fragmented->heap() == blur->heap() ||
      fragmented->size() != 3 || pool.stats().page_count != 3 ||
      pool.stats().live_slots != 8 || c->gpu().ptr != retained)
    return dx12_fail(13, "globally sufficient but split free slots must grow one whole group page");
  *fragmented = {}; // No GPU commands were recorded or submitted.
  *a = {}; // Free slots0/1/3 now suffice without contiguity.
  auto holes = pool.allocate(*state->device.Get(), 3);
  if (!holes || holes->heap() != first || pool.stats().page_count != 3)
    return dx12_fail(4, "one-page noncontiguous holes were not reused atomically");
  std::set<UINT64> distinct;
  bool reused = false;
  for (unsigned index = 0; index != 3; ++index) {
    distinct.insert(holes->gpu(index).ptr);
    reused |= holes->gpu(index).ptr == returned;
  }
  if (distinct.size() != 3 || !reused || distinct.contains(retained))
    return dx12_fail(5, "free-list duplicated slots or reused a still-live image slot");
  Dx12SrvAllocation moved(std::move(*holes));
  if (holes->size() || holes->heap() || holes->gpu().ptr || moved.size() != 3)
    return dx12_fail(6, "move constructor did not empty the old lease");
  moved = std::move(*blur); // Releases former group exactly once.
  if (blur->size() || pool.stats().live_slots != 4) return dx12_fail(7, "move assignment leaked/double-returned slots");
  moved = {}; *c = {};
  if (pool.stats().live_slots || pool.stats().page_count != 3 || pool.stats().successful_page_creations != 3)
    return dx12_fail(8, "empty pages must stay reusable at native-epoch high-water");
  pool.clear_after_retirement(); // Never submitted GPU work in this metadata/native-heap test.
  if (pool.stats().page_count || pool.stats().capacity || pool.stats().successful_page_creations != 3)
    return dx12_fail(9, "retirement failed to clear pages or lost creation history");
  for (UINT invalid : {0U, 2U, UINT_MAX}) {
    Dx12SrvDescriptorPool bad(invalid);
    if (bad.allocate(*state->device.Get(), 1) || bad.stats().page_count)
      return dx12_fail(10, "invalid private capacity allocated a native page");
  }
  for (unsigned invalid : {0U, 2U, 4U, UINT_MAX})
    if (pool.allocate(*state->device.Get(), invalid) || pool.stats().page_count)
      return dx12_fail(11, "invalid group mutated the page catalog");
  return 0;
}
