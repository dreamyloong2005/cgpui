#include "dx12_recovery_work_internal.hpp"
#include "dx12_native_retirement_internal.hpp"
#include "dx12_operation_internal.hpp"
#include "dx12_state_internal.hpp"
#include <utility>

namespace cgpui {
namespace {
NativeWindowWorkResult retire_owner(const std::shared_ptr<Dx12State>& peer,
                                  const Dx12RecoveryTickets::Lease& expected = {}) noexcept {
  std::unique_lock gate(peer->operation_mutex, std::try_to_lock);
  if (!gate.owns_lock() || peer->operation_depth != 0) return NativeWindowWorkResult::deferred;
  auto& tickets = dx12_recovery_tickets();
  const auto lease = peer->recovery_lease;
  // A stale message must not retire a replacement device, even on the same LUID.
  if ((expected && expected != lease) || !tickets.requested(lease)) return NativeWindowWorkResult::complete;
  if (peer->window_thread_id != GetCurrentThreadId()) return NativeWindowWorkResult::deferred;
  if (!peer->device) return NativeWindowWorkResult::deferred; // Attaching/destructor has not acked yet.
  const HRESULT reason = peer->device->GetDeviceRemovedReason();
  if (SUCCEEDED(reason)) {
    tickets.observed_healthy(lease); // Unaffected, not a fabricated retirement acknowledgement.
    return NativeWindowWorkResult::complete;
  }
  Dx12OperationGuard retiring(*peer);
  peer->recovery_ticket = tickets.ticket_for(lease);
  peer->removal_reason = reason;
  peer->recovery_pending = true;
  peer->failed = true;
  dx12_release_native_objects(*peer);
  return NativeWindowWorkResult::complete;
}

class RecoveryWork final : public NativeWindowWork {
 public:
  explicit RecoveryWork(std::weak_ptr<Dx12State> state) : state_(std::move(state)) {}
  NativeWindowWorkResult run() noexcept override {
    auto state = state_.lock();
    // Expiration is not an ack: the independent lease remains pending until the
    // state destructor completes its actual native release.
    return state ? retire_owner(state) : NativeWindowWorkResult::deferred;
  }
 private:
  std::weak_ptr<Dx12State> state_;
};
} // namespace

void dx12_enroll_recovery_work(const std::shared_ptr<Dx12State>& state) {
  if (!state->window_thread_id) state->window_thread_id = GetWindowThreadProcessId(state->hwnd, nullptr);
  if (state->recovery_work) return;
  state->recovery_work = std::make_shared<RecoveryWork>(state);
  state->recovery_work_token = native_window_work_register(state->hwnd, state->recovery_work);
}
void dx12_cancel_recovery_work(Dx12State& state) noexcept {
  native_window_work_cancel(state.recovery_work_token);
  state.recovery_work_token = {};
  state.recovery_work.reset();
}
void dx12_post_recovery_work(NativeWindowWorkToken token,
                             const Dx12RecoveryTickets::Lease& lease) noexcept {
  auto& tickets = dx12_recovery_tickets();
  if (!tickets.requested(lease)) return;
  if (!token) tickets.fail(lease, Dx12RecoveryTicketStatus::endpoint_unavailable);
  else if (!native_window_work_post(token)) tickets.fail(lease, Dx12RecoveryTicketStatus::post_failed);
}
void dx12_dispatch_recovery_work(Dx12State& origin) {
  auto& tickets = dx12_recovery_tickets();
  for (const auto& lease : tickets.waiting(origin.recovery_ticket)) {
    if (lease == origin.recovery_lease) continue;
    if (tickets.owner_thread(lease) == GetCurrentThreadId()) {
      auto peer = tickets.owner(lease).lock(); // Never promote a foreign state under origin's gate.
      if (!peer) continue; // Destruction is not an ack; its release path will acknowledge.
      if (retire_owner(peer, lease) == NativeWindowWorkResult::complete) continue;
      if (!tickets.endpoint(lease)) continue; // Same-owner direct retry remains legal after busy work exits.
    }
    dx12_post_recovery_work(tickets.endpoint(lease), lease);
  }
}
} // namespace cgpui
