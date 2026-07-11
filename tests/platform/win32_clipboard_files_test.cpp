#include "cgpui/platform/clipboard.hpp"
#include "win32_clipboard_test_lock.hpp"

#include <shellapi.h>

#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

std::optional<std::wstring> read_system_text() {
  if (OpenClipboard(nullptr) == FALSE) return std::nullopt;
  const HANDLE handle = GetClipboardData(CF_UNICODETEXT);
  const auto* text = handle == nullptr
      ? nullptr
      : static_cast<const wchar_t*>(GlobalLock(handle));
  const std::optional<std::wstring> result =
      text == nullptr ? std::nullopt : std::optional<std::wstring>{text};
  if (text != nullptr) GlobalUnlock(handle);
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
  const bool installed = EmptyClipboard() != FALSE &&
      SetClipboardData(CF_UNICODETEXT, handle) != nullptr;
  CloseClipboard();
  if (!installed) GlobalFree(handle);
  return installed;
}

std::optional<std::vector<std::wstring>> read_system_files() {
  if (OpenClipboard(nullptr) == FALSE) return std::nullopt;
  const HANDLE handle = GetClipboardData(CF_HDROP);
  if (handle == nullptr) {
    CloseClipboard();
    return std::nullopt;
  }
  const auto drop = reinterpret_cast<HDROP>(handle);
  const UINT count = DragQueryFileW(drop, 0xFFFFFFFFU, nullptr, 0);
  std::vector<std::wstring> files;
  files.reserve(count);
  for (UINT index = 0; index < count; ++index) {
    const UINT length = DragQueryFileW(drop, index, nullptr, 0);
    std::wstring path(static_cast<std::size_t>(length) + 1U, L'\0');
    if (DragQueryFileW(drop, index, path.data(), length + 1U) != length) {
      CloseClipboard();
      return std::nullopt;
    }
    path.resize(length);
    files.push_back(std::move(path));
  }
  CloseClipboard();
  return files;
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
  if (!clipboard_lock) return 10;
  const ClipboardRestore restore;
  std::unique_ptr<cgpui::Clipboard> clipboard =
      cgpui::create_platform_clipboard();
  if (clipboard == nullptr) return 1;

  const std::vector<std::string> paths{
      "C:\\alpha\\report.txt",
      "D:\\unicode\\\xE4\xB8\xAD\xE6\x96\x87-\xF0\x9F\x98\x80.bin",
  };
  if (!clipboard->write_files(paths)) return 2;
  const auto read_back = clipboard->read_files();
  if (!read_back || *read_back != paths) return 3;
  const auto system_files = read_system_files();
  const std::vector<std::wstring> expected{
      L"C:\\alpha\\report.txt",
      L"D:\\unicode\\\u4E2D\u6587-\U0001F600.bin",
  };
  if (!system_files || *system_files != expected) return 4;

  if (!write_system_text(L"baseline")) return 5;
  if (clipboard->write_files({})) return 6;
  const std::vector<std::string> invalid_utf8{{char(0xC3), char(0x28)}};
  if (clipboard->write_files(invalid_utf8)) return 7;
  const std::vector<std::string> embedded_nul{std::string{'a', '\0', 'b'}};
  if (clipboard->write_files(embedded_nul)) return 8;
  if (read_system_text() != std::wstring{L"baseline"}) return 9;
  return 0;
}
