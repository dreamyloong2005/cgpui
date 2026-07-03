#include "clipboard_win32_internal.hpp"

#if defined(_WIN32)
#include <cwchar>

namespace cgpui {

std::optional<std::string> Win32Clipboard::read_text() const {
  if (IsClipboardFormatAvailable(CF_UNICODETEXT) == FALSE ||
      OpenClipboard(nullptr) == FALSE) {
    return std::nullopt;
  }

  const HANDLE handle = GetClipboardData(CF_UNICODETEXT);
  if (handle == nullptr) {
    CloseClipboard();
    return std::nullopt;
  }

  const auto* text = static_cast<const wchar_t*>(GlobalLock(handle));
  if (text == nullptr) {
    CloseClipboard();
    return std::nullopt;
  }

  auto result = narrow_clipboard_text(
      std::wstring_view(text, std::wcslen(text)));
  GlobalUnlock(handle);
  CloseClipboard();
  return result;
}

} // namespace cgpui
#endif
