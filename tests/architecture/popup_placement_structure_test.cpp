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

// Popup placement math is one platform-neutral function shared by the
// backends (docs/superpowers/plans/2026-11-22-popup-windows-windows-linux-follow-up.md).
int main() {
  try {
    const auto header = read("src/platform/platform_popup_placement_internal.hpp");
    const auto source = read("src/platform/platform_popup_placement.cpp");
    const auto macos = read("src/platform/macos/macos_popup_window.mm");
    const auto macos_header = read("src/platform/macos/macos_popup_window_internal.hpp");
    const auto behavior = read("tests/platform/platform_popup_placement_test.cpp");
    const auto module = read("tests/platform/mcpp.toml");
    require(lines(header) <= 35 && lines(source) <= 70 &&
                has(header, "struct PlatformPopupFrame") &&
                has(header, "platform_popup_frame(") &&
                has(source, "PlatformPopupFrame platform_popup_frame(") &&
                !has(header, "<windows.h>") && !has(header, "AppKit") &&
                !has(source, "<windows.h>") && !has(source, "wayland"),
            "placement is a neutral leaf in src/platform");
    require(has(macos, "platform_popup_frame(") && !has(macos, "shift_into") &&
                !has(macos_header, "MacOSPopupFrame"),
            "macOS uses the shared placement instead of its own copy");
    const auto shared = module.find("[test]");
    const auto windows_only = module.find("[target.'cfg(os = \"windows\")'.test]");
    const auto entry = module.find("\"platform_popup_placement_test.cpp\"");
    require(has(behavior, "platform_popup_frame(") && entry != std::string::npos &&
                shared < entry && entry < windows_only,
            "the placement table runs on every platform");
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
