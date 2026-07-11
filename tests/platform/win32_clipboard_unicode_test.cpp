#include "cgpui/platform/clipboard.hpp"
#include "win32_clipboard_test_lock.hpp"

#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace {

std::optional<std::wstring> read_system_text() {
  if (OpenClipboard(nullptr) == FALSE) return std::nullopt;
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
  std::wstring result(text);
  GlobalUnlock(handle);
  CloseClipboard();
  return result;
}

bool write_system_text(std::wstring_view text) {
  const std::size_t bytes = (text.size() + 1U) * sizeof(wchar_t);
  HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, bytes);
  if (handle == nullptr) return false;
  void* locked = GlobalLock(handle);
  if (locked == nullptr) {
    GlobalFree(handle);
    return false;
  }
  std::memcpy(locked, text.data(), text.size() * sizeof(wchar_t));
  static_cast<wchar_t*>(locked)[text.size()] = L'\0';
  GlobalUnlock(handle);
  if (OpenClipboard(nullptr) == FALSE) {
    GlobalFree(handle);
    return false;
  }
  const bool emptied = EmptyClipboard() != FALSE;
  const bool installed = emptied &&
      SetClipboardData(CF_UNICODETEXT, handle) != nullptr;
  CloseClipboard();
  if (!installed) GlobalFree(handle);
  return installed;
}

class ClipboardRestore {
 public:
  ClipboardRestore() : original_(read_system_text()) {}
  ~ClipboardRestore() {
    if (original_) {
      (void)write_system_text(*original_);
    } else if (OpenClipboard(nullptr) != FALSE) {
      (void)EmptyClipboard();
      CloseClipboard();
    }
  }

 private:
  std::optional<std::wstring> original_;
};

} // namespace

int main() {
  const cgpui::test::Win32ClipboardTestLock clipboard_lock;
  if (!clipboard_lock) return 11;
  const ClipboardRestore restore;
  std::unique_ptr<cgpui::Clipboard> clipboard =
      cgpui::create_platform_clipboard();
  if (clipboard == nullptr) return 1;

  const std::string valid =
      std::string{"line one\r\nemoji "} + "\xF0\x9F\x98\x80";
  if (!clipboard->write_text(valid)) return 2;
  const auto read_back = clipboard->read_text();
  if (!read_back || *read_back != valid) return 3;
  const auto system_valid = read_system_text();
  if (!system_valid || *system_valid != L"line one\r\nemoji \U0001F600") {
    return 4;
  }

  if (!clipboard->write_text("") || read_system_text() != std::wstring{}) {
    return 5;
  }
  if (!write_system_text(L"baseline")) return 6;

  constexpr char invalid_utf8[]{char(0xC3), char(0x28)};
  if (clipboard->write_text(std::string_view(invalid_utf8, 2))) return 7;
  if (read_system_text() != std::wstring{L"baseline"}) return 8;

  constexpr char embedded_nul[]{'a', '\0', 'b'};
  if (clipboard->write_text(std::string_view(embedded_nul, 3))) return 9;
  if (read_system_text() != std::wstring{L"baseline"}) return 10;
  return 0;
}
