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
    const auto header = read("src/platform/win32/win32_window_client_size_internal.hpp");
    const auto source = read("src/platform/win32/win32_window_client_size.cpp");
    const auto factory = read("src/platform/win32/win32_window_factory.cpp");
    const auto module = read("tests/platform/mcpp.toml");
    require(lines(header) <= 30 && lines(source) <= 60 &&
            has(source, "GetDpiForWindow(hwnd)") &&
            has(source, "win32_outer_size_for_client("),
            "logical client sizing must stay in its focused module");
    require(has(factory, "win32_set_logical_client_size(hwnd, descriptor.size)") &&
            factory.find("apply_window_chrome(") <
                factory.find("win32_set_logical_client_size(") &&
            factory.find("win32_set_logical_client_size(") <
                factory.find("ShowWindow("),
            "the factory must size the client after the chrome frame and before showing");
    require(has(module, "\"win32_logical_window_size_test.cpp\"") &&
            has(module, "[target.'cfg(os = \"windows\")'.test]"),
            "the behavior test must be owned by its capability module");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "logical window size structure failed: " << error.what() << '\n';
    return 1;
  }
}
