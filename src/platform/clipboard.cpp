#include "clipboard_internal.hpp"

#if defined(__linux__)
#include "linux/linux_backend_selection_internal.hpp"
#endif

#include <memory>

namespace cgpui {

std::optional<std::vector<std::string>> Clipboard::read_files() const {
  return std::nullopt;
}

bool Clipboard::write_files(std::span<const std::string>) {
  return false;
}

std::unique_ptr<Clipboard> create_platform_clipboard() {
#if defined(_WIN32)
  return create_win32_clipboard();
#elif defined(__linux__)
  const auto backend = select_linux_platform_backend_from_environment();
  if (backend && *backend == LinuxPlatformBackend::x11) {
    return create_x11_clipboard();
  }
  return backend ? create_wayland_clipboard()
                 : std::make_unique<MemoryClipboard>();
#elif defined(__APPLE__)
  return create_macos_clipboard();
#else
  return std::make_unique<MemoryClipboard>();
#endif
}

} // namespace cgpui
