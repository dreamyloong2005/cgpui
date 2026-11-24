#include "dx12_command_reuse_test_support.hpp"
#include <initializer_list>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace cgpui;
using cgpui_test::require_reuse;
using Access = Dx12CommandReuseTestAccess;

namespace {
void released_parent_pending_gpu() {
  Dx12CommandReuseLifetime x;
  require_reuse(Access::begin(x, 3) && Access::create(x, 3), "pending setup");
  const auto entry = x.entry_epoch();
  require_reuse(Access::attach(x, 3, entry, 10) && Access::reserve(x, 3, 10, 20), "submitted pending entry");
  require_reuse(Access::release(x, 3, 10), "actual list release grants only P");
  Access::invalidate(x);
  // Pending polling, event wake and wait errors deliberately have no proof API.
  require_reuse(x.parent_detached() && !x.gpu_retired() && !x.can_release_entry() &&
      !Access::forget(x, 3, entry) && !Access::begin(x, 4), "P cannot release unknown GPU use");
  require_reuse(!Access::complete(x, 2, 10, 20) && !Access::complete(x, 3, 9, 20) &&
      !Access::complete(x, 3, 10, 19) && !Access::complete(x, 3, 10, 0), "stale receipts must not grant G");
  require_reuse(!Access::remove(x, 2) && !x.gpu_retired(), "other generation removal is not proof");
  require_reuse(Access::complete(x, 3, 10, 20) && x.can_release_entry(), "exact completion releases both proofs");
  require_reuse(Access::forget(x, 3, entry) && Access::begin(x, 4) && Access::create(x, 4), "retired next generation");
  const auto next = x.entry_epoch();
  require_reuse(next > entry && !Access::begin(x, 3) && !Access::forget(x, 3, entry), "no generation or entry ABA");
  require_reuse(!Access::attach(x, 4, next, 10) && Access::attach(x, 4, next, 11), "parent IDs survive generation change");
  require_reuse(!Access::release(x, 3, 10) && !Access::release(x, 4, 10), "old P receipts cannot detach new parent");
  require_reuse(!Access::reserve(x, 4, 11, 20) && Access::reserve(x, 4, 11, 21), "submission IDs survive generation change");
  require_reuse(!Access::complete(x, 3, 10, 20) && !x.gpu_retired(), "old completion cannot retire new use");
  require_reuse(Access::remove(x, 4), "confirmed current removal grants G and disables hits");
  require_reuse(x.gpu_retired() && !x.parent_detached() && !x.enabled() && !x.can_release_entry(), "removal never grants P");
  require_reuse(!Access::complete(x, 4, 11, 21) && !Access::publish(x, 4, next, 11, 21), "removed output not a successful frame");
  require_reuse(Access::release(x, 4, 11) && Access::forget(x, 4, next), "central parent-first removal ordering");
  require_reuse(!Access::create(x, 4) && Access::begin(x, 5) && Access::create(x, 5), "removed native generation stays disabled");
}
void zero_and_overflow_ids() {
  Dx12CommandReuseLifetime x;
  require_reuse(!Access::begin(x, 0) && !Access::create(x, 0), "zero identities cannot create evidence");
  require_reuse(Access::begin(x, 1), "valid identity after invalid requests");
  Access::exhaust_entry_ids(x);
  require_reuse(!Access::create(x, 1) && !x.enabled() && !x.entry_epoch(), "entry exhaustion does not wrap");
  for (const bool exhaust_parent : {false, true}) {
    Dx12CommandReuseLifetime y;
    require_reuse(Access::begin(y, 1) && Access::create(y, 1), "counter exhaustion setup");
    const auto maximum = std::numeric_limits<std::uint64_t>::max();
    const auto p = exhaust_parent ? maximum : 1, s = exhaust_parent ? 1 : maximum;
    const auto e = y.entry_epoch();
    require_reuse(Access::attach(y, 1, e, p) && Access::reserve(y, 1, p, s) &&
        Access::complete(y, 1, p, s) && Access::publish(y, 1, e, p, s) && Access::reset(y, 1, p),
        "last representable identity remains distinct");
    require_reuse(!Access::attach(y, 1, e, 0) && !y.enabled() && y.can_release_entry(),
        "exhausted parent/submission disables reuse without revoking retirement proof");
    require_reuse(Access::forget(y, 1, e), "exhaustion is fallback, not a lifetime leak");
  }
}
}
int main() {
  try { released_parent_pending_gpu(); zero_and_overflow_ids(); return 0; }
  catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
