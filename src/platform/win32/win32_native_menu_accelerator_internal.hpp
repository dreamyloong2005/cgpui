#pragma once

#include "cgpui/platform/platform_native_menu.hpp"

#include <string>

namespace cgpui {

[[nodiscard]] std::wstring win32_native_menu_display_title(
    const NativeMenuItem& item);

} // namespace cgpui
