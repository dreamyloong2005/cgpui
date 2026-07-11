#include "win32_open_url_internal.hpp"

#include "win32_internal.hpp"

#include <shellapi.h>

namespace cgpui {

std::optional<std::wstring> win32_open_url_plan(std::string_view url) {
  if (url.empty() || url.find('\0') != std::string_view::npos) return std::nullopt;
  std::wstring wide = widen(url);
  if (wide.empty()) return std::nullopt;
  return wide;
}

bool win32_shell_execute_succeeded(INT_PTR result) { return result > 32; }

PlatformOpenUrlResult win32_open_url(std::string_view url) {
  const auto plan = win32_open_url_plan(url);
  if (!plan.has_value()) {
    return {.supported = true, .backend = "win32",
            .error_message = "invalid URL"};
  }
  const INT_PTR result = reinterpret_cast<INT_PTR>(ShellExecuteW(
      nullptr, L"open", plan->c_str(), nullptr, nullptr, SW_SHOWNORMAL));
  return PlatformOpenUrlResult{
      .supported = true,
      .opened = win32_shell_execute_succeeded(result),
      .backend = "win32",
      .error_message = win32_shell_execute_succeeded(result)
          ? "" : "ShellExecuteW failed",
  };
}

} // namespace cgpui
