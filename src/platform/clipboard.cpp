#include "clipboard_internal.hpp"

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
  return create_wayland_clipboard();
#elif defined(__APPLE__)
  return create_macos_clipboard();
#else
  return std::make_unique<MemoryClipboard>();
#endif
}

} // namespace cgpui
