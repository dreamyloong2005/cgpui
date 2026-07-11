#pragma once

#include "clipboard_internal.hpp"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <optional>
#include <string>
#include <string_view>

namespace cgpui {

std::optional<std::wstring> widen_clipboard_text(std::string_view value);
std::optional<std::string> narrow_clipboard_text(std::wstring_view value);

class Win32Clipboard final : public Clipboard {
 public:
  [[nodiscard]] std::optional<std::string> read_text() const override;
  [[nodiscard]] bool write_text(std::string_view text) override;
  [[nodiscard]] std::optional<std::vector<std::string>>
  read_files() const override;
  [[nodiscard]] bool write_files(
      std::span<const std::string> paths) override;
};

} // namespace cgpui
#endif
