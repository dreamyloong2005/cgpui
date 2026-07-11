#include "clipboard_wayland_internal.hpp"

namespace cgpui {

#if defined(__linux__)
void WaylandClipboard::Connection::write_payload_to_fd(
    const std::string& payload,
    int fd) {
  errno = 0;
  const bool sent = wayland_clipboard_write_payload_incrementally(fd, payload);
  const int transfer_error = errno;
  close(fd);
  record_diagnostics(
      WaylandClipboardOperation::send,
      sent ? WaylandClipboardFailure::none
           : (transfer_error == EPIPE
                  ? WaylandClipboardFailure::receiver_closed
                  : (transfer_error == ETIMEDOUT
                         ? WaylandClipboardFailure::transfer_timeout
                         : WaylandClipboardFailure::io_error)),
      sent ? payload.size() : 0);
}
#endif

} // namespace cgpui
