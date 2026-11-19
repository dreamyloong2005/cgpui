#include <algorithm>
#include <cstdlib>
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

int main() {
  try {
    const auto leaf = read("include/cgpui/platform/platform_dpi_awareness.hpp");
    const auto application = read("include/cgpui/platform/platform_application.hpp");
    const auto aggregate = read("include/cgpui/platform/platform.hpp");
    const auto fallback = read("src/platform/platform_dpi_awareness.cpp");
    const auto native = read("src/platform/win32/win32_dpi_awareness.cpp");
    const auto native_header = read("src/platform/win32/win32_dpi_awareness_internal.hpp");
    const auto win32_application = read("src/platform/win32/win32_application.cpp");
    const auto system_application = read("src/platform/win32/win32_system_application.cpp");
    const auto factory = read("src/platform/win32/win32_window_factory.cpp");
    const auto module = read("build/xmake/capabilities/dpi_awareness_targets.lua");
    require(lines(leaf) <= 30 && has(leaf, "per_monitor_aware_v2") &&
            has(leaf, "already_set_by_process") && !has(leaf, "<windows.h>"),
            "dpi awareness leaf must stay small and platform neutral");
    require(has(application, "PlatformDpiAwarenessState dpi_awareness() const") &&
            has(aggregate, "cgpui/platform/platform_dpi_awareness.hpp"),
            "PlatformApplication must expose the awareness for diagnostics");
    require(lines(fallback) <= 20 && has(fallback, "PlatformDpiAwareness::unsupported") &&
            !has(fallback, "per_monitor_aware_v2"),
            "fallback must report unsupported instead of a fabricated declaration");
    require(lines(native) <= 70 && lines(native_header) <= 20 &&
            has(native, "SetProcessDpiAwarenessContext(") &&
            has(native, "DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2") &&
            has(native, "ERROR_ACCESS_DENIED") && has(native, "static const"),
            "win32 declaration must be focused, once-only, and tolerate host manifests");
    require(has(win32_application, "public Win32SystemApplication") &&
            lines(win32_application) <= 171 &&
            has(system_application, ": dpi_awareness_(win32_declare_dpi_awareness())") &&
            has(factory, "win32_declare_dpi_awareness()") &&
            factory.find("win32_declare_dpi_awareness()") <
                factory.find("CreateWindowExW("),
            "awareness must be declared before the first window is created");
    require(!has(win32_application, "SetProcessDpiAwarenessContext(") &&
            !has(system_application, "SetProcessDpiAwarenessContext(") &&
            !has(factory, "SetProcessDpiAwarenessContext("),
            "the declaration must stay in its focused module");
    require(has(module, "tests/platform/win32_dpi_awareness_test.cpp") &&
            has(module, "is_plat(\"windows\")"),
            "dpi awareness behavior test must be owned by its capability module");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "DPI awareness structure failed: " << error.what() << '\n';
    return 1;
  }
}
