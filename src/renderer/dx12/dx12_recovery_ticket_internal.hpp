#pragma once

#include "../../core/native_window_work_internal.hpp"
#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

namespace cgpui {
struct Dx12State;
struct Dx12RecoveryLease;
struct Dx12RecoveryTicket;

enum class Dx12RecoveryTicketStatus { pending, ready, timed_out, endpoint_unavailable, post_failed };

// CPU-only coordination. Native objects are never retained here. A lease starts
// before CreateDevice and ends only after the actual native references release.
class Dx12RecoveryTickets {
 public:
  using Clock = std::chrono::steady_clock;
  using Lease = std::shared_ptr<Dx12RecoveryLease>;
  using Ticket = std::shared_ptr<Dx12RecoveryTicket>;
  explicit Dx12RecoveryTickets(std::chrono::milliseconds timeout = std::chrono::seconds(5));
  ~Dx12RecoveryTickets();
  Dx12RecoveryTickets(const Dx12RecoveryTickets&) = delete;
  Dx12RecoveryTickets& operator=(const Dx12RecoveryTickets&) = delete;

  // Empty while an unresolved loss wave seals this adapter against new attaches.
  Lease begin_attach(std::uint64_t luid, std::weak_ptr<Dx12State> owner = {},
                     std::uint32_t owner_thread = 0, NativeWindowWorkToken token = {});
  void attached(const Lease& lease) noexcept;
  void released(const Lease& lease) noexcept;
  Ticket request(const Lease& origin, Clock::time_point now = Clock::now());
  Ticket ticket_for(const Lease& lease) const noexcept;
  bool requested(const Lease& lease) const noexcept;
  void observed_healthy(const Lease& lease) noexcept;
  void fail(const Lease& lease, Dx12RecoveryTicketStatus failure) noexcept;
  Dx12RecoveryTicketStatus poll(const Ticket& ticket, Clock::time_point now = Clock::now()) noexcept;
  std::vector<Lease> waiting(const Ticket& ticket) const;
  std::weak_ptr<Dx12State> owner(const Lease& lease) const noexcept;
  std::uint64_t identity(const Lease& lease) const noexcept;
  std::uint32_t owner_thread(const Lease& lease) const noexcept;
  NativeWindowWorkToken endpoint(const Lease& lease) const noexcept;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

Dx12RecoveryTickets& dx12_recovery_tickets();
} // namespace cgpui
