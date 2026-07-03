#include "clipboard_win32_internal.hpp"

#include <memory>

#if defined(_WIN32)
namespace cgpui {

std::unique_ptr<Clipboard> create_win32_clipboard() {
  return std::make_unique<Win32Clipboard>();
}

} // namespace cgpui
#endif
