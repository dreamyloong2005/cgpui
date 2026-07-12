#include "clipboard_wayland_diagnostics_internal.hpp"

namespace cgpui {

#if defined(__linux__)
void WaylandClipboardDiagnosticState::lock_writer() {
  while (writer_lock_.test_and_set(std::memory_order_acquire)) {}
}

void WaylandClipboardDiagnosticState::unlock_writer() {
  writer_lock_.clear(std::memory_order_release);
}

void WaylandClipboardDiagnosticState::record(
    WaylandClipboardOperation operation,
    WaylandClipboardFailure failure,
    std::size_t bytes_transferred) {
  lock_writer();
  sequence_.fetch_add(1, std::memory_order_acq_rel);
  operation_.store(operation, std::memory_order_relaxed);
  failure_.store(failure, std::memory_order_relaxed);
  bytes_transferred_.store(bytes_transferred, std::memory_order_relaxed);
  sequence_.fetch_add(1, std::memory_order_release);
  unlock_writer();
}

void WaylandClipboardDiagnosticState::record_ownership(
    bool owns_selection,
    std::size_t owned_payload_bytes) {
  lock_writer();
  sequence_.fetch_add(1, std::memory_order_acq_rel);
  owns_selection_.store(owns_selection, std::memory_order_relaxed);
  owned_payload_bytes_.store(owned_payload_bytes, std::memory_order_relaxed);
  ownership_revision_.fetch_add(1, std::memory_order_relaxed);
  sequence_.fetch_add(1, std::memory_order_release);
  unlock_writer();
}

WaylandClipboardDiagnostics WaylandClipboardDiagnosticState::snapshot() const {
  while (true) {
    const std::uint64_t before = sequence_.load(std::memory_order_acquire);
    if ((before & 1U) != 0) continue;
    const WaylandClipboardDiagnostics result{
        .operation = operation_.load(std::memory_order_relaxed),
        .failure = failure_.load(std::memory_order_relaxed),
        .bytes_transferred = bytes_transferred_.load(std::memory_order_relaxed),
        .revision = before / 2U,
        .owns_selection = owns_selection_.load(std::memory_order_relaxed),
        .owned_payload_bytes =
            owned_payload_bytes_.load(std::memory_order_relaxed),
        .ownership_revision =
            ownership_revision_.load(std::memory_order_relaxed),
    };
    if (sequence_.load(std::memory_order_acquire) == before) return result;
  }
}
#endif

} // namespace cgpui
