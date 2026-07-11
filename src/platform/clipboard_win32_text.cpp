#include "clipboard_win32_internal.hpp"

#if defined(_WIN32)
#include <climits>
#include <utility>

namespace cgpui {

std::optional<std::wstring> widen_clipboard_text(std::string_view value) {
  if (value.find('\0') != std::string_view::npos || value.size() > INT_MAX) {
    return std::nullopt;
  }
  if (value.empty()) {
    return std::wstring{};
  }

  const auto required = MultiByteToWideChar(
      CP_UTF8,
      MB_ERR_INVALID_CHARS,
      value.data(),
      static_cast<int>(value.size()),
      nullptr,
      0);
  if (required <= 0) {
    return std::nullopt;
  }

  std::wstring result(static_cast<std::size_t>(required), L'\0');
  const int written = MultiByteToWideChar(
      CP_UTF8,
      MB_ERR_INVALID_CHARS,
      value.data(),
      static_cast<int>(value.size()),
      result.data(),
      required);
  return written == required
      ? std::optional<std::wstring>{std::move(result)}
      : std::nullopt;
}

std::optional<std::string> narrow_clipboard_text(std::wstring_view value) {
  if (value.find(L'\0') != std::wstring_view::npos || value.size() > INT_MAX) {
    return std::nullopt;
  }
  if (value.empty()) {
    return std::string{};
  }

  const auto required = WideCharToMultiByte(
      CP_UTF8,
      WC_ERR_INVALID_CHARS,
      value.data(),
      static_cast<int>(value.size()),
      nullptr,
      0,
      nullptr,
      nullptr);
  if (required <= 0) {
    return std::nullopt;
  }

  std::string result(static_cast<std::size_t>(required), '\0');
  const int written = WideCharToMultiByte(
      CP_UTF8,
      WC_ERR_INVALID_CHARS,
      value.data(),
      static_cast<int>(value.size()),
      result.data(),
      required,
      nullptr,
      nullptr);
  return written == required
      ? std::optional<std::string>{std::move(result)}
      : std::nullopt;
}

} // namespace cgpui
#endif
