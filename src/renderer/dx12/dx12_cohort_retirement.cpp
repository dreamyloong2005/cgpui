#include "dx12_device_cohort_internal.hpp"
#include "dx12_native_retirement_internal.hpp"
#include "dx12_operation_internal.hpp"
#include "dx12_state_internal.hpp"
#include <mutex>

namespace cgpui {
void dx12_retire_removed_cohort_peers(Dx12State& origin) {
  const auto peers = dx12_device_cohort_peers(origin);
  for (const auto& weak : peers) {
    auto peer = weak.lock();
    if (!peer || peer.get() == &origin) continue;
    std::unique_lock gate(peer->operation_mutex, std::try_to_lock);
    // Recursive locking alone does not prove idleness on this same thread.
    if (!gate.owns_lock() || peer->operation_depth != 0 || !peer->device) continue;
    Dx12OperationGuard retiring(*peer);
    // A pending old participant must never retire another participant's new,
    // healthy device. Same LUID alone is not evidence of the same lost epoch.
    const HRESULT reason = peer->device->GetDeviceRemovedReason();
    if (SUCCEEDED(reason)) continue;
    peer->removal_reason = reason;
    peer->recovery_pending = true;
    peer->failed = true;
    dx12_release_native_objects(*peer);
    // CPU frames/caches/captures and generation stay untouched. Even orphaned
    // states are purged, but only a later live owner's operation can rebuild.
  }
}
} // namespace cgpui
