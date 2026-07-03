#include "clipboard_win32_internal.hpp"

#if defined(_WIN32)
#include <cstring>

namespace cgpui {

bool Win32Clipboard::write_text(std::string_view text) {
  const std::wstring wide_text = widen_clipboard_text(text);
  if (!text.empty() && wide_text.empty()) {
    return false;
  }

  const std::size_t byte_size =
      (wide_text.size() + 1U) * sizeof(wchar_t);
  HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, byte_size);
  if (handle == nullptr) {
    return false;
  }

  void* locked = GlobalLock(handle);
  if (locked == nullptr) {
    GlobalFree(handle);
    return false;
  }

  std::memcpy(locked, wide_text.c_str(), byte_size);
  GlobalUnlock(handle);

  if (OpenClipboard(nullptr) == FALSE) {
    GlobalFree(handle);
    return false;
  }

  if (EmptyClipboard() == FALSE) {
    CloseClipboard();
    GlobalFree(handle);
    return false;
  }

  if (SetClipboardData(CF_UNICODETEXT, handle) == nullptr) {
    CloseClipboard();
    GlobalFree(handle);
    return false;
  }

  CloseClipboard();
  return true;
}

} // namespace cgpui
#endif
