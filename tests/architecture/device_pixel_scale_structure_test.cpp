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
    const auto header = read("src/renderer/vulkan/vulkan_device_pixels_internal.hpp");
    const auto source = read("src/renderer/vulkan/vulkan_device_pixels.cpp");
    const auto renderer = read("src/renderer/vulkan/vulkan_renderer.cpp");
    const auto ime = read("src/platform/win32/win32_window_ime_placement.cpp");
    const auto module = read("tests/renderer/mcpp.toml") +
      read("tests/platform/mcpp.toml");
    require(lines(header) <= 30 && lines(source) <= 130 &&
            has(header, "vulkan_to_device_pixels(") &&
            !has(source, "<vulkan/vulkan.h>"),
            "logical-to-device conversion must stay a focused, Vulkan-free module");
    require(has(renderer, "vulkan_to_device_pixels(draw, state_->scale())"),
            "every recorded frame draw must be converted to device pixels");
    require(has(ime, "state_.scale.value"),
            "IME placement must convert logical rects to physical pixels");
    require(has(module, "\"vulkan_scale_pixel_test.cpp\"") &&
            has(module, "\"vulkan_device_pixels_test.cpp\"") &&
            has(module, "\"win32_ime_placement_scale_test.cpp\""),
            "the behavior tests must be owned by the capability module");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "device pixel scale structure failed: " << error.what() << '\n';
    return 1;
  }
}
