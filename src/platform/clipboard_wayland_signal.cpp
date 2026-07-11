#include "clipboard_wayland_internal.hpp"

#include <csignal>
#include <pthread.h>

namespace cgpui {

#if defined(__linux__)
ssize_t wayland_clipboard_write_without_sigpipe(
    int fd,
    const void* data,
    std::size_t size) {
  sigset_t sigpipe_set{};
  sigemptyset(&sigpipe_set);
  sigaddset(&sigpipe_set, SIGPIPE);

  sigset_t pending{};
  const bool sigpipe_was_pending =
      sigpending(&pending) == 0 && sigismember(&pending, SIGPIPE) == 1;
  sigset_t old_mask{};
  const int mask_error = pthread_sigmask(SIG_BLOCK, &sigpipe_set, &old_mask);
  if (mask_error != 0) {
    errno = mask_error;
    return -1;
  }

  const ssize_t result = write(fd, data, size);
  const int write_error = errno;
  if (result == -1 && write_error == EPIPE && !sigpipe_was_pending &&
      sigpending(&pending) == 0 && sigismember(&pending, SIGPIPE) == 1) {
    timespec no_wait{};
    while (sigtimedwait(&sigpipe_set, nullptr, &no_wait) == -1 &&
           errno == EINTR) {}
  }
  (void)pthread_sigmask(SIG_SETMASK, &old_mask, nullptr);
  errno = write_error;
  return result;
}
#endif

} // namespace cgpui
