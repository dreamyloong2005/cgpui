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
  const auto root = std::getenv("CGPUI_SOURCE_ROOT");
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
    const auto header = read("src/platform/win32/win32_window_proc_background_internal.hpp");
    const auto source = read("src/platform/win32/win32_window_proc_background.cpp");
    const auto proc = read("src/platform/win32/win32_window_proc.cpp");
    const auto backdrop = read("src/platform/win32/win32_window_backdrop.cpp");
    const auto factory = read("src/platform/win32/win32_window_factory.cpp");
    const auto module = read("build/xmake/capabilities/backdrop_erase_targets.lua");
    require(lines(header) <= 20 && lines(source) <= 35 &&
            has(source, "WM_ERASEBKGND") && has(source, "BLACK_BRUSH") &&
            has(source, "win32_window_backdrop_property"),
            "the backdrop erase handler must stay focused and conditional");
    require(has(proc, "win32_window_proc_handle_background(hwnd, message, wparam, result)"),
            "the window procedure must route WM_ERASEBKGND to the handler");
    require(has(backdrop, "RDW_INVALIDATE | RDW_ERASE") &&
            has(backdrop, "frame_extended_ = plan.extend_frame"),
            "extending the frame must erase the redirection surface again");
    require(!has(factory, "hbrBackground"),
            "opaque windows keep the null class brush");
    require(has(module, "tests/platform/win32_backdrop_erase_test.cpp"),
            "the behavior test must be owned by its capability module");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "backdrop erase structure failed: " << error.what() << '\n';
    return 1;
  }
}
