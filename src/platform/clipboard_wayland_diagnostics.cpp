#include "clipboard_wayland_diagnostics_internal.hpp"

namespace cgpui {

#if defined(__linux__)
void WaylandClipboardDiagnosticState::record(
    WaylandClipboardOperation operation,
    WaylandClipboardFailure failure,
    std::size_t bytes_transferred) {
  sequence_.fetch_add(1, std::memory_order_acq_rel);
  operation_.store(operation, std::memory_order_relaxed);
  failure_.store(failure, std::memory_order_relaxed);
  bytes_transferred_.store(bytes_transferred, std::memory_order_relaxed);
  sequence_.fetch_add(1, std::memory_order_release);
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
    };
    if (sequence_.load(std::memory_order_acquire) == before) return result;
  }
}
#endif

} // namespace cgpui
