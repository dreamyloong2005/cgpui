#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const std::string& name) {
  auto root = std::filesystem::path(__FILE__);
  if (root.is_relative()) root = std::filesystem::absolute(root);
  root = root.parent_path().parent_path().parent_path();
  std::ifstream file(root / name);
  if (!file) throw std::runtime_error("missing shadow owner: " + name);
  return {std::istreambuf_iterator<char>(file), {}};
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
    for (const char* leaf : {"box_shadow_vertex", "box_shadow_geometry", "box_shadow_parameters"}) {
      const auto source = read(std::string("include/cgpui/renderer/planning/") + leaf + ".hpp");
      require(std::count(source.begin(), source.end(), '\n') <= 50 &&
                  !has(source, "vulkan") && !has(source, "windows.h") &&
                  !has(source, "src/") && !has(source, "for ("),
              "shadow planning leaves must be small native-free declarations");
    }
    const auto geometry = read("src/renderer/planning/box_shadow_geometry.cpp");
    const auto parameters = read("src/renderer/planning/box_shadow_parameters.cpp");
    require(std::count(geometry.begin(), geometry.end(), '\n') <= 140 &&
                std::count(parameters.begin(), parameters.end(), '\n') <= 40 &&
                !has(geometry, "vulkan_") && !has(parameters, "vulkan_") &&
                has(geometry, "rounded_rect_radii_internal.hpp") &&
                has(geometry, "resolve_rounded_rect_radii(") &&
                has(geometry, "resolve_effective_clip_rect("),
            "neutral shadow planning must own algorithms and reuse neutral radii/clipping");
    const auto compatibility = read("src/renderer/vulkan/vulkan_box_shadow_geometry.cpp");
    require(std::count(compatibility.begin(), compatibility.end(), '\n') <= 35 &&
                has(compatibility, "return build_box_shadow_geometry(") &&
                has(compatibility, "return box_shadow_sigma(") &&
                has(compatibility, "return box_shadow_spread_radius(") &&
                !has(compatibility, "for (") && !has(compatibility, "std::isfinite"),
            "Vulkan shadow geometry must only forward to the neutral owner");
    const auto header = read("src/renderer/vulkan/vulkan_box_shadow_geometry_internal.hpp");
    require(has(header, "using VulkanBoxShadowVertex = BoxShadowVertex;") &&
                has(header, "using VulkanBoxShadowGeometry = BoxShadowGeometry;"),
            "Vulkan shadow types must remain compatible aliases");
    const auto manifest = read("tests/renderer-neutral/mcpp.toml");
    require(!has(manifest, "renderer-vulkan") && !has(manifest, "renderer-dx12"),
            "neutral shadow tests cannot link native backends");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
