#include "dx12_srv_pool_test_support.hpp"
#include <set>

using namespace cgpui;
using namespace cgpui_test;
int main() {
  auto state = texture_upload_state();
  if (!state) return dx12_fail(1, "SRV growth device unavailable");
  Dx12SrvDescriptorPool pool;
  std::vector<Dx12SrvAllocation> leases;
  leases.reserve(4097);
  std::set<UINT64> handles;
  for (unsigned index = 0; index != 4097; ++index) {
    auto lease = pool.allocate(*state->device.Get(), 1);
    if (!lease || !handles.insert(lease->gpu().ptr).second ||
        lease->heap()->GetDesc().NumDescriptors != 4096)
      return dx12_fail(2, "default4096 real heap or handle uniqueness changed");
    leases.push_back(std::move(*lease));
  }
  if (pool.stats().page_count != 2 || pool.stats().capacity != 8192 || pool.stats().live_slots != 4097 ||
      pool.stats().successful_page_creations != 2 || leases[0].heap() != leases[4095].heap() ||
      leases[0].heap() == leases[4096].heap()) return dx12_fail(3, "4097th live slot failed stable-page growth");
  const auto old = leases[41].gpu().ptr;
  leases[41] = {};
  auto reused = pool.allocate(*state->device.Get(), 1);
  if (!reused || reused->gpu().ptr != old || pool.stats().successful_page_creations != 2)
    return dx12_fail(4, "default page hole was not reused");
  *reused = {}; leases.clear(); pool.clear_after_retirement();
  return pool.stats().page_count == 0 ? 0 : dx12_fail(5, "default pages leaked at retirement");
}
