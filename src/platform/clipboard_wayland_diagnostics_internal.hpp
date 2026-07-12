#pragma once

#include "cgpui/platform/clipboard.hpp"

#include <atomic>

namespace cgpui {

#if defined(__linux__)
class WaylandClipboardDiagnosticState {
 public:
  void record(
      WaylandClipboardOperation operation,
      WaylandClipboardFailure failure,
      std::size_t bytes_transferred);
  void record_ownership(bool owns_selection, std::size_t owned_payload_bytes);
  [[nodiscard]] WaylandClipboardDiagnostics snapshot() const;

 private:
  void lock_writer();
  void unlock_writer();

  std::atomic_flag writer_lock_ = ATOMIC_FLAG_INIT;
  std::atomic_uint64_t sequence_{0};
  std::atomic<WaylandClipboardOperation> operation_{
      WaylandClipboardOperation::none};
  std::atomic<WaylandClipboardFailure> failure_{
      WaylandClipboardFailure::none};
  std::atomic_size_t bytes_transferred_{0};
  std::atomic_bool owns_selection_{false};
  std::atomic_size_t owned_payload_bytes_{0};
  std::atomic_uint64_t ownership_revision_{0};
};
#endif

} // namespace cgpui
