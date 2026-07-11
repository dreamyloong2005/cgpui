#include "clipboard_wayland_internal.hpp"

namespace cgpui {

#if defined(__linux__)
std::optional<std::string> WaylandClipboard::Connection::read_offer_payload(
    const std::string& mime_type) {
  if (selection_offer_ == nullptr || selection_offer_->offer == nullptr) {
    return std::nullopt;
  }

  int pipe_fds[2] = {-1, -1};
  if (pipe2(pipe_fds, O_CLOEXEC) == -1) {
    return std::nullopt;
  }

  wl_data_offer_receive(selection_offer_->offer, mime_type.c_str(), pipe_fds[1]);
  if (wl_display_flush(display_) == -1) {
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    return std::nullopt;
  }

  close(pipe_fds[1]);
  pipe_fds[1] = -1;

  std::string payload;
  auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (std::chrono::steady_clock::now() < deadline) {
    pollfd descriptor{
        .fd = pipe_fds[0],
        .events = POLLIN | POLLHUP,
        .revents = 0,
    };
    const int ready = poll(&descriptor, 1, 50);
    if (ready == -1 && errno == EINTR) {
      continue;
    }
    if (ready <= 0) {
      continue;
    }

    char buffer[4096];
    const auto bytes_read = read(pipe_fds[0], buffer, sizeof(buffer));
    if (bytes_read > 0) {
      payload.append(buffer, static_cast<std::size_t>(bytes_read));
      deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
      continue;
    }
    if (bytes_read == -1 && errno == EINTR) continue;
    close(pipe_fds[0]);
    return bytes_read == 0 ? std::optional<std::string>{std::move(payload)}
                           : std::nullopt;
  }

  close(pipe_fds[0]);
  return std::nullopt;
}
#endif

} // namespace cgpui
