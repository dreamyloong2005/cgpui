#include "cgpui/platform/clipboard.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#ifdef _WIN32
namespace {

std::wstring widen_for_test(std::string_view text) {
  if (text.empty()) {
    return {};
  }

  const int required = MultiByteToWideChar(
      CP_UTF8,
      0,
      text.data(),
      static_cast<int>(text.size()),
      nullptr,
      0);
  if (required <= 0) {
    return {};
  }

  std::wstring result(static_cast<std::size_t>(required), L'\0');
  MultiByteToWideChar(
      CP_UTF8,
      0,
      text.data(),
      static_cast<int>(text.size()),
      result.data(),
      required);
  return result;
}

std::string narrow_for_test(std::wstring_view text) {
  if (text.empty()) {
    return {};
  }

  const int required = WideCharToMultiByte(
      CP_UTF8,
      0,
      text.data(),
      static_cast<int>(text.size()),
      nullptr,
      0,
      nullptr,
      nullptr);
  if (required <= 0) {
    return {};
  }

  std::string result(static_cast<std::size_t>(required), '\0');
  WideCharToMultiByte(
      CP_UTF8,
      0,
      text.data(),
      static_cast<int>(text.size()),
      result.data(),
      required,
      nullptr,
      nullptr);
  return result;
}

std::optional<std::string> read_system_clipboard_text_for_test() {
  if (OpenClipboard(nullptr) == FALSE) {
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

  const std::wstring_view wide_text(text);
  auto result = narrow_for_test(wide_text);
  GlobalUnlock(handle);
  CloseClipboard();
  return result;
}

bool write_system_clipboard_text_for_test(std::string_view text) {
  const auto wide_text = widen_for_test(text);
  const std::size_t byte_size = (wide_text.size() + 1U) * sizeof(wchar_t);
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

class SystemClipboardRestore {
 public:
  SystemClipboardRestore() : original_(read_system_clipboard_text_for_test()) {}

  ~SystemClipboardRestore() {
    if (original_.has_value()) {
      (void)write_system_clipboard_text_for_test(*original_);
    } else if (OpenClipboard(nullptr) != FALSE) {
      (void)EmptyClipboard();
      CloseClipboard();
    }
  }

 private:
  std::optional<std::string> original_;
};

} // namespace
#endif

namespace {

int test_memory_clipboard_round_trips_utf8_text() {
  cgpui::MemoryClipboard clipboard;
  if (clipboard.read_text().has_value()) {
    return 1;
  }

  if (!clipboard.write_text("hello")) {
    return 2;
  }
  const auto first = clipboard.read_text();
  if (!first || *first != std::string_view{"hello"}) {
    return 3;
  }

  if (!clipboard.write_text("\xE4\xB8\xAD")) {
    return 4;
  }
  const auto second = clipboard.read_text();
  return second && *second == std::string_view{"\xE4\xB8\xAD"} ? 0 : 5;
}

int test_platform_clipboard_uses_text_clipboard_contract() {
  const auto clipboard = cgpui::create_platform_clipboard();
  if (clipboard == nullptr) {
    return 6;
  }

  if (!clipboard->write_text("platform text")) {
    return 7;
  }
  const auto text = clipboard->read_text();
  return text && *text == std::string_view{"platform text"} ? 0 : 8;
}

#ifdef _WIN32
int test_win32_platform_clipboard_reads_system_utf8_text() {
  const SystemClipboardRestore restore;
  if (!write_system_clipboard_text_for_test("system \xE4\xB8\xAD")) {
    return 9;
  }

  const auto clipboard = cgpui::create_platform_clipboard();
  if (clipboard == nullptr) {
    return 10;
  }

  const auto text = clipboard->read_text();
  return text && *text == std::string_view{"system \xE4\xB8\xAD"} ? 0 : 11;
}

int test_win32_platform_clipboard_writes_system_utf8_text() {
  const SystemClipboardRestore restore;
  const auto clipboard = cgpui::create_platform_clipboard();
  if (clipboard == nullptr) {
    return 12;
  }

  if (!clipboard->write_text("written \xE4\xB8\xAD")) {
    return 13;
  }

  const auto text = read_system_clipboard_text_for_test();
  return text && *text == std::string_view{"written \xE4\xB8\xAD"} ? 0 : 14;
}
#endif

} // namespace

int main() {
  if (const int result = test_memory_clipboard_round_trips_utf8_text();
      result != 0) {
    return result;
  }
  if (const int result = test_platform_clipboard_uses_text_clipboard_contract();
      result != 0) {
    return result;
  }
#ifdef _WIN32
  if (const int result =
          test_win32_platform_clipboard_reads_system_utf8_text();
      result != 0) {
    return result;
  }
  if (const int result =
          test_win32_platform_clipboard_writes_system_utf8_text();
      result != 0) {
    return result;
  }
#endif
#if defined(__linux__)
  {
    cgpui::WaylandClipboard clipboard;
    if (clipboard.support() != cgpui::WaylandClipboardSupport::unsupported) {
      return 15;
    }
    if (!clipboard.write_text("wayland fallback \xE4\xB8\xAD")) {
      return 16;
    }
    const auto text = clipboard.read_text();
    if (!text || *text != std::string_view{"wayland fallback \xE4\xB8\xAD"}) {
      return 17;
    }
  }
  {
    cgpui::WaylandClipboard clipboard(cgpui::WaylandClipboardOptions{
        .data_device_manager_available = true,
        .seat_available = false});
    if (clipboard.support() != cgpui::WaylandClipboardSupport::no_seat) {
      return 18;
    }
    if (!clipboard.write_text("wayland no seat")) {
      return 19;
    }
    const auto text = clipboard.read_text();
    if (!text || *text != std::string_view{"wayland no seat"}) {
      return 20;
    }
  }
  {
    cgpui::WaylandClipboard clipboard(cgpui::WaylandClipboardOptions{
        .data_device_manager_available = true,
        .seat_available = true});
    if (clipboard.support() != cgpui::WaylandClipboardSupport::available) {
      return 21;
    }
    if (!clipboard.write_text("wayland available")) {
      return 22;
    }
    const auto text = clipboard.read_text();
    if (!text || *text != std::string_view{"wayland available"}) {
      return 23;
    }
  }
  {
    const auto clipboard = cgpui::create_platform_clipboard();
    const auto* wayland =
        dynamic_cast<const cgpui::WaylandClipboard*>(clipboard.get());
    if (wayland == nullptr) {
      return 24;
    }
    if (wayland->support() != cgpui::WaylandClipboardSupport::unsupported) {
      return 25;
    }
  }
#endif
  return 0;
}
