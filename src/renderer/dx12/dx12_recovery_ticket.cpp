#include "dx12_recovery_ticket_internal.hpp"
#include <algorithm>
#include <limits>
#include <mutex>
#include <utility>

namespace cgpui {
struct Dx12RecoveryLease {
  std::uint64_t identity = 0, luid = 0;
  std::uint64_t released_before_identity = 0;
  std::uint32_t owner_thread = 0;
  NativeWindowWorkToken endpoint;
  std::weak_ptr<Dx12State> owner;
  std::weak_ptr<Dx12RecoveryTicket> ticket;
  bool attaching = true, released = false, healthy = false;
};
struct Dx12RecoveryTicket {
  std::uint64_t luid = 0;
  Dx12RecoveryTickets::Clock::time_point deadline;
  Dx12RecoveryTicketStatus failure = Dx12RecoveryTicketStatus::pending;
  std::vector<Dx12RecoveryTickets::Lease> members;
};
struct Dx12RecoveryTickets::Impl {
  std::mutex mutex;
  std::chrono::milliseconds timeout;
  std::uint64_t next_identity = 1;
  std::vector<Lease> leases;
  std::vector<Ticket> waves;
};
namespace {
bool unresolved(const Dx12RecoveryTickets::Lease& lease) {
  return !lease->released && !lease->healthy;
}
bool ready(const Dx12RecoveryTickets::Ticket& ticket) {
  return std::none_of(ticket->members.begin(), ticket->members.end(), [&](const auto& lease) {
    return lease->ticket.lock() == ticket && unresolved(lease);
  });
}
} // namespace

Dx12RecoveryTickets::Dx12RecoveryTickets(std::chrono::milliseconds timeout)
    : impl_(std::make_unique<Impl>()) { impl_->timeout = timeout; }
Dx12RecoveryTickets::~Dx12RecoveryTickets() = default;

Dx12RecoveryTickets::Lease Dx12RecoveryTickets::begin_attach(
    std::uint64_t luid, std::weak_ptr<Dx12State> owner,
    std::uint32_t owner_thread, NativeWindowWorkToken token) {
  std::lock_guard lock(impl_->mutex);
  std::erase_if(impl_->waves, ready);
  std::erase_if(impl_->leases, [](const Lease& lease) { return lease->released; });
  for (const auto& wave : impl_->waves) if (wave->luid == luid) return {};
  if (impl_->next_identity == std::numeric_limits<std::uint64_t>::max()) return {};
  auto lease = std::make_shared<Dx12RecoveryLease>();
  lease->identity = impl_->next_identity++;
  lease->luid = luid;
  lease->owner = std::move(owner);
  lease->owner_thread = owner_thread;
  lease->endpoint = token;
  impl_->leases.push_back(lease);
  return lease;
}
void Dx12RecoveryTickets::attached(const Lease& lease) noexcept {
  if (!lease) return;
  std::lock_guard lock(impl_->mutex);
  lease->attaching = false;
}
void Dx12RecoveryTickets::released(const Lease& lease) noexcept {
  if (!lease) return;
  std::lock_guard lock(impl_->mutex);
  // Freeze the first actual release boundary. Later cleanup may acknowledge
  // this same lease again, after unrelated replacement devices have attached.
  if (!lease->released) lease->released_before_identity = impl_->next_identity;
  lease->attaching = false;
  lease->released = true;
}
Dx12RecoveryTickets::Ticket Dx12RecoveryTickets::request(const Lease& origin, Clock::time_point now) {
  if (!origin) return {};
  std::lock_guard lock(impl_->mutex);
  for (const auto& wave : impl_->waves) {
    if (wave->luid != origin->luid || ready(wave)) continue;
    if (!origin->released && (origin->ticket.lock() != wave || origin->healthy)) {
      // A genuinely live newly removed lease may be newer than an old released
      // request's membership boundary. Join it without extending the deadline.
      auto members = wave->members;
      for (const auto& lease : impl_->leases)
        if (lease->luid == origin->luid && !lease->released &&
            std::find(members.begin(), members.end(), lease) == members.end()) members.push_back(lease);
      wave->members = std::move(members); // Allocate before mutating existing acknowledgements.
      for (const auto& lease : wave->members) {
        if (lease->released) continue;
        lease->healthy = false;
        lease->ticket = wave;
      }
    }
    return wave;
  }
  if (auto existing = origin->ticket.lock(); existing && !origin->healthy) return existing;
  auto wave = std::make_shared<Dx12RecoveryTicket>();
  wave->luid = origin->luid;
  wave->deadline = now + impl_->timeout;
  for (const auto& lease : impl_->leases) {
    if (lease->luid != origin->luid || lease->released) continue;
    // A late recovery of an already-purged lease cannot declare subsequently
    // attached healthy replacements removed. Pre-release/in-flight owners stay.
    if (origin->released && lease->identity >= origin->released_before_identity) continue;
    // Stage allocation first: a failed allocation cannot leave a half request.
    wave->members.push_back(lease);
  }
  impl_->waves.push_back(wave);
  for (const auto& lease : wave->members) { lease->healthy = false; lease->ticket = wave; }
  origin->ticket = wave;
  return wave;
}
Dx12RecoveryTickets::Ticket Dx12RecoveryTickets::ticket_for(const Lease& lease) const noexcept {
  if (!lease) return {};
  std::lock_guard lock(impl_->mutex);
  return lease->ticket.lock();
}
bool Dx12RecoveryTickets::requested(const Lease& lease) const noexcept {
  if (!lease) return false;
  std::lock_guard lock(impl_->mutex);
  return !lease->ticket.expired() && unresolved(lease);
}
void Dx12RecoveryTickets::observed_healthy(const Lease& lease) noexcept {
  if (!lease) return;
  std::lock_guard lock(impl_->mutex);
  // An attachment in flight cannot be mistaken for a completed healthy device.
  if (!lease->attaching && !lease->ticket.expired()) lease->healthy = true;
}
void Dx12RecoveryTickets::fail(const Lease& lease, Dx12RecoveryTicketStatus failure) noexcept {
  if (!lease || failure == Dx12RecoveryTicketStatus::pending || failure == Dx12RecoveryTicketStatus::ready) return;
  std::lock_guard lock(impl_->mutex);
  if (auto wave = lease->ticket.lock(); wave && unresolved(lease) &&
      wave->failure == Dx12RecoveryTicketStatus::pending) wave->failure = failure;
}
Dx12RecoveryTicketStatus Dx12RecoveryTickets::poll(const Ticket& ticket, Clock::time_point now) noexcept {
  if (!ticket) return Dx12RecoveryTicketStatus::endpoint_unavailable;
  std::lock_guard lock(impl_->mutex);
  if (ready(ticket)) return Dx12RecoveryTicketStatus::ready;
  if (ticket->failure != Dx12RecoveryTicketStatus::pending) return ticket->failure;
  if (now >= ticket->deadline) ticket->failure = Dx12RecoveryTicketStatus::timed_out;
  return ticket->failure;
}
std::vector<Dx12RecoveryTickets::Lease> Dx12RecoveryTickets::waiting(const Ticket& ticket) const {
  std::vector<Lease> result;
  if (!ticket) return result;
  std::lock_guard lock(impl_->mutex);
  for (const auto& lease : ticket->members)
    if (lease->ticket.lock() == ticket && unresolved(lease)) result.push_back(lease);
  return result;
}
std::weak_ptr<Dx12State> Dx12RecoveryTickets::owner(const Lease& lease) const noexcept {
  return lease ? lease->owner : std::weak_ptr<Dx12State>{};
}
std::uint64_t Dx12RecoveryTickets::identity(const Lease& lease) const noexcept {
  return lease ? lease->identity : 0;
}
std::uint32_t Dx12RecoveryTickets::owner_thread(const Lease& lease) const noexcept {
  return lease ? lease->owner_thread : 0;
}
NativeWindowWorkToken Dx12RecoveryTickets::endpoint(const Lease& lease) const noexcept {
  return lease ? lease->endpoint : NativeWindowWorkToken{};
}
Dx12RecoveryTickets& dx12_recovery_tickets() {
  static Dx12RecoveryTickets tickets;
  return tickets;
}
} // namespace cgpui
