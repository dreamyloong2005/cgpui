#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
std::string read(const char* path) {
  const auto root = CGPUI_SOURCE_ROOT;
  std::ifstream stream(std::filesystem::path(root ? root : ".") / path);
  if (!stream) throw std::runtime_error(std::string("missing focused module: ") + path);
  return {std::istreambuf_iterator<char>(stream), {}};
}
void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
bool has(const std::string& source, std::string_view text) {
  return source.find(text) != std::string::npos;
}
std::size_t lines(const std::string& source) {
  return std::count(source.begin(), source.end(), '\n') + 1;
}
} // namespace

// Win32 native context menus: a focused source, the shared menu tree with
// a popup root, and a deferred command message
// (docs/superpowers/plans/2026-11-22-popup-windows-windows-linux-follow-up.md).
int main() {
  try {
    const auto source = read("src/platform/win32/win32_window_context_menu.cpp");
    const auto tree = read("src/platform/win32/win32_native_menu_tree.cpp");
    const auto tree_header = read("src/platform/win32/win32_native_menu_tree_internal.hpp");
    const auto proc_header = read("src/platform/win32/win32_window_proc_command_internal.hpp");
    const auto proc = read("src/platform/win32/win32_window_proc_command.cpp");
    const auto window = read("src/platform/win32/win32_window_internal.hpp");
    const auto fallback = read("src/platform/platform_window_popup.cpp");
    const auto behavior = read("tests/platform/win32_context_menu_test.cpp");
    const auto module = read("tests/platform/mcpp.toml");
    require(lines(source) <= 90 && has(source, "Win32NativeMenuTree::build_popup(") &&
                has(source, "Win32NativeMenuCommandMap::build(") &&
                has(source, "TPM_RETURNCMD") && has(source, "ClientToScreen(") &&
                has(source, "SetForegroundWindow(") && has(source, "WM_NULL") &&
                has(source, "win32_context_menu_command_message") &&
                has(source, "IsWindow(hwnd)"),
            "the context menu reuses the menu tree and defers its command");
    require(has(tree, "CreatePopupMenu(), model") && has(tree_header, "build_popup(") &&
                has(proc_header, "win32_context_menu_command_message") &&
                has(proc, "context_menu_command()") &&
                has(window, "show_context_menu(") && has(fallback, "unsupported"),
            "Win32 overrides show_context_menu; other backends keep the fallback");
    require(lines(behavior) <= 180 && has(behavior, "EndMenu()") &&
                has(behavior, "MN_GETHMENU") && has(behavior, "MFS_GRAYED") &&
                has(behavior, "MFS_CHECKED") && has(behavior, "MFT_SEPARATOR") &&
                has(behavior, "NativeMenuCommandSource::context_menu") &&
                has(module, "\"win32_context_menu_test.cpp\""),
            "win32_context_menu_test covers cancel, choices and item state");
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
