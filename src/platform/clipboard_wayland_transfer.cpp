#include "clipboard_wayland_internal.hpp"

namespace cgpui {

#if defined(__linux__)
bool wayland_clipboard_write_payload_incrementally(
    int fd,
    std::string_view payload) {
  std::size_t written = 0;
  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (written < payload.size()) {
    const auto count = wayland_clipboard_write_without_sigpipe(
        fd, payload.data() + written, payload.size() - written);
    if (count > 0) {
      written += static_cast<std::size_t>(count);
      deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
      continue;
    }
    if (count == -1 && errno == EINTR) continue;
    if (count != -1 || (errno != EAGAIN && errno != EWOULDBLOCK)) return false;

    while (std::chrono::steady_clock::now() < deadline) {
      pollfd descriptor{.fd = fd, .events = POLLOUT, .revents = 0};
      const int ready = poll(&descriptor, 1, 50);
      if (ready == -1 && errno == EINTR) continue;
      if (ready > 0 && (descriptor.revents & POLLOUT) != 0) break;
      if (ready > 0 && (descriptor.revents & (POLLERR | POLLHUP)) != 0) {
        return false;
      }
    }
    if (std::chrono::steady_clock::now() >= deadline) {
      errno = ETIMEDOUT;
      return false;
    }
  }
  return true;
}
#endif

} // namespace cgpui
