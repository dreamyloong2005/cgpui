#include "dx12_srv_pool_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  auto state = texture_upload_state();
  if (!state) return dx12_fail(1, "SRV failure device unavailable");
  SrvFailingHeapApi api;
  Dx12SrvDescriptorPool pool(4, &api);
  auto first = pool.allocate(*state->device.Get(), 3);
  if (!first) return dx12_fail(2, "initial real native group unavailable");
  auto* heap = first->heap();
  const auto handle = first->gpu().ptr;
  api.fail_on = 2;
  auto failed = pool.allocate(*state->device.Get(), 3);
  if (failed || failed.error().message.find("80070005") == std::string::npos ||
      failed.error().message.find("SRV descriptor page allocation") == std::string::npos ||
      pool.stats().live_slots != 3 || pool.stats().capacity != 4 || pool.stats().page_count != 1 ||
      pool.stats().successful_page_creations != 1 || api.successes != 1 ||
      first->heap() != heap || first->gpu().ptr != handle)
    return dx12_fail(3, "failed grouped growth changed old views or lost original HRESULT");
  auto last = pool.allocate(*state->device.Get(), 1);
  if (!last || last->heap() != heap || api.attempts != 2 || pool.stats().live_slots != 4)
    return dx12_fail(4, "failed blur3 partially reserved the existing page's last free slot");
  api.fail_on = 0;
  auto retry = pool.allocate(*state->device.Get(), 3); // Unsubmitted module, not failed renderer resurrection.
  if (!retry || retry->heap() == heap || retry->size() != 3 ||
      api.successes != 2 || pool.stats().successful_page_creations != 2 || pool.stats().live_slots != 7)
    return dx12_fail(5, "independent allocator retry failed atomic native growth");
  return 0;
}
