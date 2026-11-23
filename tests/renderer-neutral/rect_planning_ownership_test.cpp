#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

namespace {
std::filesystem::path root() {
  auto path = std::filesystem::path(__FILE__);
  if (path.is_relative()) path = std::filesystem::absolute(path);
  return path.parent_path().parent_path().parent_path();
}
std::string read(const std::filesystem::path& path) {
  std::ifstream stream(root() / path);
  return {std::istreambuf_iterator<char>(stream), {}};
}
}

int main() {
  for (const auto name : {"rect_geometry", "clip_scissor", "composition"}) {
    const auto source = read(std::string("include/cgpui/renderer/planning/") + name + ".hpp");
    if (source.empty() || source.find("vulkan") != std::string::npos ||
        source.find("windows.h") != std::string::npos) return 1;
  }
  for (const auto name : {"solid_rect_geometry", "rounded_rect_geometry", "clip_scissor",
                          "composition_transform", "composition_opacity"}) {
    const auto source = read(std::string("src/renderer/planning/") + name + ".cpp");
    if (source.empty() || source.find("vulkan_") != std::string::npos ||
        source.find("VkRect") != std::string::npos) return 2;
    const auto compatibility = read(std::string("src/renderer/vulkan/vulkan_") + name + ".cpp");
    if (compatibility.empty() || compatibility.find("std::floor") != std::string::npos ||
        compatibility.find("geometry.vertices") != std::string::npos) return 3;
  }
  const auto manifest = read("tests/renderer-neutral/mcpp.toml");
  if (manifest.find("renderer-vulkan") != std::string::npos ||
      manifest.find("renderer-dx12") != std::string::npos) return 4;
  return 0;
}
