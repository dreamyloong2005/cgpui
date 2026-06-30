#include "cgpui/platform/clipboard.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <cstring>
#include <cwchar>
#include <memory>
#include <string>
#include <string_view>

namespace cgpui {
namespace {

#if defined(_WIN32)
std::wstring widen_clipboard_text(std::string_view value) {
  if (value.empty()) {
    return {};
  }

  const auto required = MultiByteToWideChar(
      CP_UTF8,
      0,
      value.data(),
      static_cast<int>(value.size()),
      nullptr,
      0);
  if (required <= 0) {
    return {};
  }

  std::wstring result(static_cast<std::size_t>(required), L'\0');
  MultiByteToWideChar(
      CP_UTF8,
      0,
      value.data(),
      static_cast<int>(value.size()),
      result.data(),
      required);
  return result;
}

std::string narrow_clipboard_text(std::wstring_view value) {
  if (value.empty()) {
    return {};
  }

  const auto required = WideCharToMultiByte(
      CP_UTF8,
      0,
      value.data(),
      static_cast<int>(value.size()),
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
      value.data(),
      static_cast<int>(value.size()),
      result.data(),
      required,
      nullptr,
      nullptr);
  return result;
}

class Win32Clipboard final : public Clipboard {
 public:
  [[nodiscard]] std::optional<std::string> read_text() const override {
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

  [[nodiscard]] bool write_text(std::string_view text) override {
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
};
#endif

} // namespace

std::optional<std::string> MemoryClipboard::read_text() const {
  return text_;
}

bool MemoryClipboard::write_text(std::string_view text) {
  text_ = std::string(text);
  return true;
}

#if defined(__linux__)
WaylandClipboard::WaylandClipboard() = default;

WaylandClipboard::WaylandClipboard(WaylandClipboardOptions options)
    : support_(
          !options.data_device_manager_available
              ? WaylandClipboardSupport::unsupported
              : (options.seat_available ? WaylandClipboardSupport::available
                                        : WaylandClipboardSupport::no_seat)) {}

std::optional<std::string> WaylandClipboard::read_text() const {
  return fallback_.read_text();
}

bool WaylandClipboard::write_text(std::string_view text) {
  return fallback_.write_text(text);
}

WaylandClipboardSupport WaylandClipboard::support() const {
  return support_;
}
#endif

std::unique_ptr<Clipboard> create_platform_clipboard() {
#if defined(_WIN32)
  return std::make_unique<Win32Clipboard>();
#elif defined(__linux__)
  return std::make_unique<WaylandClipboard>();
#else
  return std::make_unique<MemoryClipboard>();
#endif
}

} // namespace cgpui
