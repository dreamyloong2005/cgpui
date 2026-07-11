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
  [[nodiscard]] WaylandClipboardDiagnostics snapshot() const;

 private:
  std::atomic_uint64_t sequence_{0};
  std::atomic<WaylandClipboardOperation> operation_{
      WaylandClipboardOperation::none};
  std::atomic<WaylandClipboardFailure> failure_{
      WaylandClipboardFailure::none};
  std::atomic_size_t bytes_transferred_{0};
};
#endif

} // namespace cgpui
