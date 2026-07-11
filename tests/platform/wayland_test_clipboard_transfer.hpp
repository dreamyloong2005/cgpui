#pragma once

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <string_view>
#include <thread>

#include <unistd.h>

namespace cgpui::test {

inline std::size_t fill_wayland_test_clipboard_pipe(int fd) {
  std::array<char, 4096> filler{};
  std::size_t filled = 0;
  while (true) {
    const auto count = write(fd, filler.data(), filler.size());
    if (count > 0) {
      filled += static_cast<std::size_t>(count);
      continue;
    }
    if (count == -1 && errno == EINTR) continue;
    return filled;
  }
}

inline bool write_wayland_test_clipboard_payload(
    int fd,
    std::string_view payload,
    std::size_t chunk_size,
    std::chrono::milliseconds chunk_delay) {
  const std::size_t transfer_size =
      chunk_size == 0 ? payload.size() : chunk_size;
  std::size_t written = 0;
  while (written < payload.size()) {
    const std::size_t remaining = payload.size() - written;
    const std::size_t requested = std::min(transfer_size, remaining);
    const auto count = write(fd, payload.data() + written, requested);
    if (count == -1 && errno == EINTR) continue;
    if (count <= 0) return false;
    written += static_cast<std::size_t>(count);
    if (written < payload.size() && chunk_delay.count() > 0) {
      std::this_thread::sleep_for(chunk_delay);
    }
  }
  return true;
}

} // namespace cgpui::test
