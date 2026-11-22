#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const std::filesystem::path& path) {
  std::ifstream stream(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!stream) throw std::runtime_error("missing planning owner: " + path.string());
  return {std::istreambuf_iterator<char>(stream), {}};
}
bool has(const std::string& source, const char* token) {
  return source.find(token) != std::string::npos;
}
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
}

int main() {
  try {
    const auto frame = read("include/cgpui/renderer/renderer_frame.hpp");
    require(!has(frame, "vulkan_") && has(frame, "planning.hpp"),
            "renderer frame must not declare backend-owned CPU planning");
    const auto package = read("packages/renderer/mcpp.toml");
    require(has(package, "src/renderer/planning/*.cpp") && !has(package, "renderer-vulkan"),
            "CPU planners must link from the renderer-neutral package");
    const auto tests = read("tests/renderer-neutral/mcpp.toml");
    require(has(tests, "cgpui.renderer =") && !has(tests, "renderer-vulkan") &&
                !has(tests, "renderer-metal") && !has(tests, "cgpui.ui ="),
            "neutral planner tests must not borrow implementation from a backend or UI");
    for (const auto* leaf : {"image_uploads", "glyph_uploads", "glyph_resources",
                             "glyph_geometry", "draw_geometry", "command_reports"}) {
      const auto source = read(std::string("include/cgpui/renderer/planning/") + leaf + ".hpp");
      require(!has(source, "vulkan_") && !has(source, "inline ") &&
                  std::count(source.begin(), source.end(), '\n') <= 85,
              "neutral planning leaves contain declarations, not backend logic");
    }
    for (const auto& entry : std::filesystem::directory_iterator(
             std::filesystem::path(CGPUI_SOURCE_ROOT) / "src/renderer/planning")) {
      const auto source = read(entry.path());
      require(!has(source, "vulkan_internal") && !has(source, "<vulkan/") &&
                  !has(source, "cgpui/ui/"), "planning must not depend on native backend or UI");
      if (!entry.path().filename().string().starts_with("compat_")) {
        require(!has(source, "vulkan_") && !has(source, "Vulkan"),
                "legacy Vulkan names belong only to forwarding compatibility sources");
      } else {
        require(!has(source, "for (") && !has(source, "while (") &&
                    std::count(source.begin(), source.end(), '\n') <= 130,
                "compatibility wrappers must only forward to their neutral owner");
      }
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
