#include "dx12_command_reuse_test_support.hpp"
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

using namespace cgpui;
using cgpui_test::require_reuse;
using Access = Dx12CommandReuseTestAccess;
template<class T> concept PublicProof = requires(T& x) { x.completed(1, 2, 3); };
template<class T> concept PublicInvalidation = requires(T& x) { x.invalidate(); };
static_assert(!PublicProof<Dx12CommandReuseLifetime> && !PublicInvalidation<Dx12CommandReuseLifetime>);
static_assert(!std::is_copy_constructible_v<Dx12CommandReuseLifetime>);
static_assert(!std::is_move_constructible_v<Dx12CommandReuseLifetime>);
static_assert(noexcept(Access::reserve(std::declval<Dx12CommandReuseLifetime&>(), 1, 2, 3)));

namespace {
void fresh_then_two_hits() {
  Dx12CommandReuseLifetime x;
  require_reuse(!x.enabled() && !x.can_release_entry() && !x.can_reuse_entry(), "empty ledger is inert");
  require_reuse(Access::begin(x, 7) && Access::create(x, 7), "create first generation entry");
  const auto entry = x.entry_epoch();
  require_reuse(entry && x.can_release_entry() && !x.can_reuse_entry(), "closed alone is not published");
  for (std::uint64_t iteration = 0; iteration != 3; ++iteration) {
    const auto parent = 101 + iteration, submission = 201 + iteration;
    require_reuse(Access::attach(x, 7, entry, parent), "attach exactly one current parent");
    require_reuse(x.gpu_retired() && !x.parent_detached() && !x.can_release_entry(), "unsubmitted parent holds P");
    require_reuse(!Access::attach(x, 7, entry, parent + 1), "second outstanding parent rejected");
    require_reuse(Access::reserve(x, 7, parent, submission), "pre-arm before native forwarding");
    require_reuse(!x.gpu_retired() && !x.can_reuse_entry(), "pre-arm conservatively clears G");
    require_reuse(!Access::reserve(x, 7, parent, submission + 1), "parent cannot be submitted twice");
    require_reuse(!Access::reset(x, 7, parent), "pending work cannot reset allocator/list pair");
    require_reuse(!Access::publish(x, 7, entry, parent, submission), "pending frame is not published");
    require_reuse(Access::complete(x, 7, parent, submission), "matching completed submission grants G");
    require_reuse(!x.can_release_entry() && !x.can_reuse_entry(), "G alone must not grant P");
    require_reuse(Access::publish(x, 7, entry, parent, submission), "publish after all external success gates");
    require_reuse(!Access::publish(x, 7, entry, parent, submission), "publication must not duplicate");
    require_reuse(!x.can_release_entry(), "success/Present is not parent detachment");
    require_reuse(Access::reset(x, 7, parent), "actual list reset detaches completed parent");
    require_reuse(x.can_release_entry() && x.can_reuse_entry(), "G and P allow next hit");
    require_reuse(x.entry_epoch() == entry, "fresh plus two hits retain exact entry epoch");
  }
  Access::invalidate(x);
  require_reuse(!x.can_reuse_entry() && x.can_release_entry(), "invalidation revokes hits only");
  require_reuse(Access::forget(x, 7, entry), "owner can forget after native release");
  require_reuse(Access::create(x, 7) && x.entry_epoch() > entry, "new entry identity is monotonic");
}
void unsubmitted_and_completed_failures() {
  Dx12CommandReuseLifetime x;
  require_reuse(Access::begin(x, 1) && Access::create(x, 1), "create failure candidate");
  auto entry = x.entry_epoch();
  require_reuse(Access::attach(x, 1, entry, 1), "attach before possible Close failure");
  Access::invalidate(x); // Native Close failure is NOT simulated by this CPU test.
  require_reuse(x.gpu_retired() && !x.parent_detached() && !Access::forget(x, 1, entry),
                "invalid unsubmitted recording retains entry until parent abandoned");
  require_reuse(Access::release(x, 1, 1) && Access::forget(x, 1, entry), "release parent first");
  require_reuse(Access::create(x, 1), "new independent candidate");
  entry = x.entry_epoch();
  require_reuse(Access::attach(x, 1, entry, 2) && Access::reserve(x, 1, 2, 1) &&
      Access::complete(x, 1, 2, 1), "completed recording before Present/publication error");
  // Allocator Reset/failed list Reset/Close/Present provide no ledger event.
  Access::invalidate(x);
  require_reuse(!x.parent_detached() && !Access::forget(x, 1, entry), "completion plus error grants no P");
  require_reuse(!Access::publish(x, 1, entry, 2, 1), "failed success gates cannot publish");
  require_reuse(Access::release(x, 1, 2) && Access::forget(x, 1, entry), "explicit abandonment releases P");
}
void unpublished_cannot_be_reexecuted() {
  Dx12CommandReuseLifetime x;
  require_reuse(Access::begin(x, 1) && Access::create(x, 1), "unpublished setup");
  const auto e = x.entry_epoch();
  require_reuse(Access::attach(x, 1, e, 1) && Access::reserve(x, 1, 1, 1) &&
      Access::complete(x, 1, 1, 1) && Access::reset(x, 1, 1), "complete without success publication");
  require_reuse(!x.can_reuse_entry() && !Access::attach(x, 1, e, 2), "both proofs do not substitute for success");
  require_reuse(Access::forget(x, 1, e), "unpublished safe entry may be discarded");
}
void publication_belongs_to_current_attempt() {
  enum class Outcome { unsubmitted_abandoned, completed_unpublished, explicitly_failed };
  for (const auto outcome : {Outcome::unsubmitted_abandoned, Outcome::completed_unpublished,
                            Outcome::explicitly_failed}) {
    Dx12CommandReuseLifetime x;
    require_reuse(Access::begin(x, 5) && Access::create(x, 5), "prime published fresh entry");
    const auto e = x.entry_epoch();
    require_reuse(Access::attach(x, 5, e, 10) && Access::reserve(x, 5, 10, 20) &&
        Access::complete(x, 5, 10, 20) && Access::publish(x, 5, e, 10, 20) &&
        Access::reset(x, 5, 10) && x.can_reuse_entry(), "fresh publication permits one next attempt");
    require_reuse(!Access::attach(x, 5, e, 10) && x.can_reuse_entry(),
        "rejected stale attachment must not clear current successful publication");
    require_reuse(Access::attach(x, 5, e, 11), "attach next attempt after successful fresh");
    require_reuse(!Access::publish(x, 5, e, 10, 20) && !Access::publish(x, 5, e, 11, 20),
        "prior publication cannot be replayed onto an unsubmitted attempt");
    if (outcome != Outcome::unsubmitted_abandoned) {
      require_reuse(Access::reserve(x, 5, 11, 21) && Access::complete(x, 5, 11, 21),
          "new submitted attempt completes independently of publication");
      require_reuse(!Access::publish(x, 5, e, 10, 21) && !Access::publish(x, 5, e, 11, 20),
          "publication must identify both exact current parent and submission");
    }
    // Model only: no native failure is injected. completed_unpublished also models
    // a failed success gate WITHOUT invalidate, so old publication cannot rescue
    // it. Real native owners must still invalidate every failed attempt explicitly.
    if (outcome == Outcome::explicitly_failed) {
      Access::invalidate(x);
      require_reuse(!Access::publish(x, 5, e, 11, 21), "explicit failure rejects current publication");
    }
    const auto detached = outcome == Outcome::unsubmitted_abandoned
        ? Access::release(x, 5, 11) : Access::reset(x, 5, 11);
    require_reuse(detached && x.gpu_retired() && x.parent_detached() && x.can_release_entry(),
        "unpublished or failed attempt remains safely releasable after G and P");
    require_reuse(!x.can_reuse_entry() && !Access::attach(x, 5, e, 12),
        "previous fresh success must not authorize reuse after an unpublished attempt");
    require_reuse(!Access::publish(x, 5, e, 11, 21) && !x.can_reuse_entry(),
        "late publication cannot resurrect an already detached attempt");
    require_reuse(Access::forget(x, 5, e), "failed eligibility does not block safe disposal");
  }
}
}
int main() {
  try {
    fresh_then_two_hits(); unsubmitted_and_completed_failures();
    unpublished_cannot_be_reexecuted(); publication_belongs_to_current_attempt(); return 0;
  }
  catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
