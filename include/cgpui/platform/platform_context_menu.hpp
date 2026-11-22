#pragma once

#include "cgpui/platform/platform_native_menu.hpp"

#include <optional>
#include <string>

namespace cgpui {

// Result of PlatformWindow::show_context_menu. The call returns when the
// menu closes; a chosen command is also delivered as a NativeMenuCommand
// with source context_menu. Backends without native context menus report
// supported = false, and the app keeps its in-window menu.
struct PlatformContextMenuResult {
  bool supported = false;
  bool shown = false;
  std::optional<std::string> action_name;
  std::string backend;
  std::string reason;
};

} // namespace cgpui
