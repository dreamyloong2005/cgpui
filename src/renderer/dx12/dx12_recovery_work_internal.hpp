#pragma once

#include "dx12_recovery_ticket_internal.hpp"
#include "../../core/native_window_work_internal.hpp"

namespace cgpui {
struct Dx12State;

// Enroll after HWND configuration and before native creation. Unmanaged HWNDs
// remain supported, but cannot accept foreign-owner automatic retirement.
void dx12_enroll_recovery_work(const std::shared_ptr<Dx12State>& state);
void dx12_cancel_recovery_work(Dx12State& state) noexcept;
void dx12_post_recovery_work(NativeWindowWorkToken token,
                             const Dx12RecoveryTickets::Lease& lease) noexcept;
// Never waits for a peer or invokes a foreign owner's callback inline.
void dx12_dispatch_recovery_work(Dx12State& origin);
} // namespace cgpui
