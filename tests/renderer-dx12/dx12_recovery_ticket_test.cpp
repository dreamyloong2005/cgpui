#include "../../src/renderer/dx12/dx12_recovery_ticket_internal.hpp"
#include <iostream>
#include <stdexcept>

using namespace cgpui;
using Status = Dx12RecoveryTicketStatus;
using namespace std::chrono_literals;

namespace {
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
void attaching_and_destruction() {
  Dx12RecoveryTickets tickets(50ms);
  const auto now = Dx12RecoveryTickets::Clock::time_point{};
  auto owner = std::shared_ptr<Dx12State>(nullptr, [](Dx12State*) {});
  auto origin = tickets.begin_attach(7), peer = tickets.begin_attach(7, owner, 42, {11, 12});
  auto partial = tickets.begin_attach(7), other = tickets.begin_attach(9);
  tickets.attached(origin);
  tickets.attached(peer);
  auto wave = tickets.request(origin, now);
  require(wave && wave == tickets.request(peer, now + 40ms), "one loss wave must coalesce");
  require(tickets.waiting(wave).size() == 3, "attaching references must join the barrier");
  require(!tickets.begin_attach(7) && tickets.begin_attach(9), "seal must be adapter-local");
  require(tickets.owner_thread(peer) == 42 && tickets.endpoint(peer).work == 12,
          "dispatch metadata must be immutable lease metadata");
  tickets.released(origin);
  owner.reset();
  require(tickets.owner(peer).expired() && tickets.requested(peer), "weak expiration is not release acknowledgement");
  tickets.observed_healthy(partial);
  require(tickets.waiting(wave).size() == 2, "in-flight CreateDevice cannot be declared healthy");
  tickets.attached(partial);
  tickets.observed_healthy(partial);
  require(tickets.poll(wave, now + 49ms) == Status::pending, "destructor still owns native references");
  tickets.released(peer); // Only after the owner's native references actually release.
  require(tickets.poll(wave, now + 49ms) == Status::ready, "all actual acknowledgements should unblock");
  auto replacement = tickets.begin_attach(7);
  require(replacement && tickets.identity(replacement) != tickets.identity(origin), "native lease identity cannot be reused");
  tickets.released(replacement);
  tickets.released(partial);
  tickets.released(other);
}
void bounded_failure() {
  Dx12RecoveryTickets tickets(20ms);
  const auto now = Dx12RecoveryTickets::Clock::time_point{};
  auto origin = tickets.begin_attach(1), parked = tickets.begin_attach(1);
  tickets.attached(origin);
  tickets.attached(parked);
  auto wave = tickets.request(origin, now);
  tickets.released(origin);
  for (int step = 1; step < 20; ++step) {
    require(tickets.request(parked, now + std::chrono::milliseconds(step)) == wave, "retry changed ticket");
    require(tickets.request(origin, now + std::chrono::milliseconds(step)) == wave,
            "released origin request changed the pending wave or deadline");
    require(tickets.poll(wave, now + std::chrono::milliseconds(step)) == Status::pending, "early timeout");
  }
  require(tickets.poll(wave, now + 20ms) == Status::timed_out, "repeated wakes extended fixed deadline");
  require(tickets.poll(wave, now + 200ms) == Status::timed_out && !tickets.begin_attach(1),
          "timeout must not fabricate release or admit another native reference");
  tickets.released(parked);
  require(tickets.poll(wave, now + 201ms) == Status::ready, "real later destruction permits explicit retry");
  auto next = tickets.begin_attach(1), bad_endpoint = tickets.begin_attach(1);
  tickets.attached(next);
  tickets.attached(bad_endpoint);
  auto failed = tickets.request(next, now + 300ms);
  tickets.released(next);
  tickets.fail(bad_endpoint, Status::post_failed);
  require(tickets.poll(failed, now + 301ms) == Status::post_failed, "queue failure was disguised as pending");
  tickets.fail(bad_endpoint, Status::endpoint_unavailable);
  require(tickets.poll(failed, now + 302ms) == Status::post_failed, "first concrete failure must be retained");
  tickets.released(bad_endpoint);
}
void opaque_owner_without_endpoint() {
  Dx12RecoveryTickets tickets(20ms);
  const auto now = Dx12RecoveryTickets::Clock::time_point{};
  auto origin = tickets.begin_attach(1), inaccessible = tickets.begin_attach(1);
  tickets.attached(origin);
  tickets.attached(inaccessible);
  auto wave = tickets.request(origin, now);
  tickets.released(origin);
  require(tickets.owner(inaccessible).expired() && tickets.requested(inaccessible),
          "stack/unregistered owners must still hold a real native lease");
  tickets.fail(inaccessible, Status::endpoint_unavailable);
  require(tickets.poll(wave, now) == Status::endpoint_unavailable && !tickets.begin_attach(1),
          "unavailable dispatch cannot fabricate retirement");
  tickets.released(inaccessible);
  require(tickets.poll(wave, now) == Status::ready, "explicit native release must unblock even without shared owner");
}
void unaffected_and_new_wave() {
  Dx12RecoveryTickets tickets(20ms);
  const auto now = Dx12RecoveryTickets::Clock::time_point{};
  auto removed = tickets.begin_attach(1), healthy = tickets.begin_attach(1);
  tickets.attached(removed);
  tickets.attached(healthy);
  auto first = tickets.request(removed, now);
  tickets.released(removed);
  tickets.observed_healthy(healthy);
  require(tickets.poll(first, now) == Status::ready && !tickets.requested(healthy),
          "actual healthy peers are unaffected, not purged");
  auto replacement = tickets.begin_attach(1);
  tickets.attached(replacement);
  auto second = tickets.request(healthy, now + 1ms);
  require(second != first && tickets.waiting(second).size() == 2, "later actual loss needs a fresh wave");
  require(tickets.poll(first, now + 2ms) == Status::ready, "new wave retroactively changed prior completion");
  require(tickets.request(removed, now + 2ms) == second, "old rebuilding participant missed the newer attach barrier");
  tickets.released(healthy);
  tickets.released(replacement);
  require(tickets.poll(second, now + 3ms) == Status::ready, "second wave never completed");
}
void released_before_wave_does_not_enlist_replacement() {
  Dx12RecoveryTickets tickets(20ms);
  const auto now = Dx12RecoveryTickets::Clock::time_point{};
  auto origin = tickets.begin_attach(1), retired_peer = tickets.begin_attach(1);
  tickets.attached(origin);
  tickets.attached(retired_peer);
  tickets.released(retired_peer); // Native purge happened before a removal wave was requested.
  auto first = tickets.request(origin, now);
  tickets.released(origin);
  require(tickets.poll(first, now) == Status::ready, "old native references were not released");
  auto replacement = tickets.begin_attach(1);
  tickets.attached(replacement);
  tickets.released(retired_peer); // A repeated ack must not move the original release boundary.
  auto late = tickets.request(retired_peer, now + 1ms);
  require(tickets.waiting(late).empty() && !tickets.requested(replacement) &&
          tickets.poll(late, now + 1ms) == Status::ready,
          "already-retired old lease enlisted a subsequently attached replacement");
  tickets.released(replacement);
}
void released_without_wave_keeps_preexisting_blockers() {
  Dx12RecoveryTickets tickets(20ms);
  const auto now = Dx12RecoveryTickets::Clock::time_point{};
  auto origin = tickets.begin_attach(1), old_peer = tickets.begin_attach(1);
  tickets.attached(origin);
  tickets.attached(old_peer);
  tickets.released(origin);
  auto newer = tickets.begin_attach(1);
  tickets.attached(newer);
  auto wave = tickets.request(origin, now);
  const auto waiting = tickets.waiting(wave);
  require(waiting.size() == 1 && waiting.front() == old_peer && !tickets.requested(newer),
          "released origin must retain old blockers without enlisting later native attachments");
  require(tickets.poll(wave, now) == Status::pending && !tickets.begin_attach(1),
          "old unresolved native owner was replaced by fabricated completion");
  tickets.released(old_peer);
  require(tickets.poll(wave, now + 1ms) == Status::ready, "real old blocker release must unblock");
  tickets.released(newer);
}
void late_live_loss_joins_existing_deadline() {
  Dx12RecoveryTickets tickets(20ms);
  const auto now = Dx12RecoveryTickets::Clock::time_point{};
  auto retired = tickets.begin_attach(1), attaching = tickets.begin_attach(1);
  tickets.attached(retired);
  tickets.released(retired);
  auto newer = tickets.begin_attach(1);
  tickets.attached(newer);
  auto wave = tickets.request(retired, now);
  require(tickets.waiting(wave).size() == 1 && tickets.waiting(wave).front() == attaching,
          "release boundary omitted a preexisting in-flight attachment");
  tickets.observed_healthy(attaching);
  require(tickets.poll(wave, now + 1ms) == Status::pending, "in-flight attachment was fabricated healthy");
  require(tickets.request(newer, now + 19ms) == wave && tickets.requested(newer) &&
          tickets.waiting(wave).size() == 2, "new live loss was not joined to the unresolved wave");
  require(tickets.poll(wave, now + 20ms) == Status::timed_out,
          "new live loss extended the existing absolute deadline");
  tickets.released(attaching);
  require(tickets.poll(wave, now + 21ms) == Status::timed_out,
          "new live native lease did not retain its own release obligation");
  tickets.released(newer);
  require(tickets.poll(wave, now + 22ms) == Status::ready, "actual complete release did not unblock joined wave");
}
} // namespace

int main() {
  try {
    attaching_and_destruction(); bounded_failure(); opaque_owner_without_endpoint();
    unaffected_and_new_wave(); released_before_wave_does_not_enlist_replacement();
    released_without_wave_keeps_preexisting_blockers(); late_live_loss_joins_existing_deadline(); return 0;
  }
  catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
