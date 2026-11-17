#pragma once

namespace cgpui {

// An extra file descriptor serviced by the Wayland event loop on the UI
// thread, used for process-wide services such as the desktop portal D-Bus
// connection. The loop re-reads the descriptor every iteration because the
// source may connect lazily while the loop is running.
class WaylandEventSource {
 public:
  virtual ~WaylandEventSource() = default;

  // The descriptor to poll for input, or -1 while the source is inactive.
  [[nodiscard]] virtual int descriptor() const = 0;
  // True when already-received data is queued, so the loop must not block.
  [[nodiscard]] virtual bool has_pending() const = 0;
  // Reads available input without blocking and dispatches queued data.
  virtual void dispatch() = 0;
};

} // namespace cgpui
