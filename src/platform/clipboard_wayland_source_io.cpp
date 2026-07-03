#include "clipboard_wayland_internal.hpp"

namespace cgpui {

#if defined(__linux__)
void WaylandClipboard::Connection::write_payload_to_fd(
    const std::string& payload,
    int fd) {
  std::size_t written = 0;
  while (written < payload.size()) {
    const auto count = write(
        fd,
        payload.data() + written,
        static_cast<unsigned int>(payload.size() - written));
    if (count == -1 && errno == EINTR) {
      continue;
    }
    if (count <= 0) {
      break;
    }
    written += static_cast<std::size_t>(count);
  }
  close(fd);
}
#endif

} // namespace cgpui
