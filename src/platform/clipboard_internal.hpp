#pragma once

#include "cgpui/platform/clipboard.hpp"

#include <memory>

namespace cgpui {

std::unique_ptr<Clipboard> create_win32_clipboard();
std::unique_ptr<Clipboard> create_wayland_clipboard();
std::unique_ptr<Clipboard> create_macos_clipboard();

} // namespace cgpui
