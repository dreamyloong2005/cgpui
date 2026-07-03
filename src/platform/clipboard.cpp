#include "clipboard_internal.hpp"

#include <memory>

namespace cgpui {

std::unique_ptr<Clipboard> create_platform_clipboard() {
#if defined(_WIN32)
  return create_win32_clipboard();
#elif defined(__linux__)
  return create_wayland_clipboard();
#else
  return std::make_unique<MemoryClipboard>();
#endif
}

} // namespace cgpui
