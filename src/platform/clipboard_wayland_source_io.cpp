#include "clipboard_wayland_internal.hpp"

namespace cgpui {

#if defined(__linux__)
void WaylandClipboard::Connection::write_payload_to_fd(
    const std::string& payload,
    int fd) {
  (void)wayland_clipboard_write_payload_incrementally(fd, payload);
  close(fd);
}
#endif

} // namespace cgpui
