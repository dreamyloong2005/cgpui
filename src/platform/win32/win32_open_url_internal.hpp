#pragma once

#include "cgpui/platform/platform_open_url.hpp"

#include <windows.h>

#include <optional>
#include <string>
#include <string_view>

namespace cgpui {

[[nodiscard]] std::optional<std::wstring> win32_open_url_plan(
    std::string_view url);
[[nodiscard]] bool win32_shell_execute_succeeded(INT_PTR result);
[[nodiscard]] PlatformOpenUrlResult win32_open_url(std::string_view url);

} // namespace cgpui
