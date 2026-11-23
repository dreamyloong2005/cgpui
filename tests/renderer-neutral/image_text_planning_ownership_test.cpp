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
  if (!file) throw std::runtime_error("missing planning owner: " + name);
  return {std::istreambuf_iterator<char>(file), {}};
}
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
bool has(const std::string& source, const char* token) {
  return source.find(token) != std::string::npos;
}
}

int main() {
  try {
    for (const char* name : {"image_vertex", "image_geometry", "image_color",
                             "text_vertex", "text_geometry", "text_positioning"}) {
      const auto source = read(std::string("include/cgpui/renderer/planning/") + name + ".hpp");
      require(std::count(source.begin(), source.end(), '\n') <= 40 &&
                  !has(source, "vulkan") && !has(source, "windows.h") &&
                  !has(source, "src/") && !has(source, "for ("),
              "public planning leaves must remain small and renderer-neutral");
    }
    for (const char* name : {"image_geometry", "image_color", "text_geometry", "text_positioning"}) {
      const auto source = read(std::string("src/renderer/planning/") + name + ".cpp");
      require(std::count(source.begin(), source.end(), '\n') <= 150 &&
                  !has(source, "vulkan") && !has(source, "Vulkan") &&
                  !has(source, "dx12") && !has(source, "cgpui/ui/"),
              "CPU image/text algorithms belong only to neutral implementation modules");
    }
    const auto image = read("src/renderer/vulkan/vulkan_image_vertex_buffer.cpp");
    const auto text = read("src/renderer/vulkan/vulkan_text_vertex_buffer.cpp");
    const auto tint = read("src/renderer/vulkan/vulkan_image_color.cpp");
    const auto positioning = read("src/renderer/vulkan/vulkan_text_positioning.cpp");
    require(has(image, "return build_image_vertices(") && !has(image, "image_uv_bounds(") &&
                !has(image, "append_image_quad_vertices("),
            "Vulkan image buffer must forward CPU geometry rather than duplicate it");
    require(has(text, "return build_text_vertices(") && !has(text, "append_text_quad_vertices("),
            "Vulkan text buffer must forward CPU geometry rather than duplicate it");
    require(has(tint, "return resolve_image_tint(") && !has(tint, "value_or("),
            "Vulkan tint must forward to its neutral owner");
    require(has(positioning, "return position_text_bounds(") && !has(positioning, "std::round"),
            "Vulkan text positioning must forward to its neutral owner");
    require(has(read("src/renderer/vulkan/vulkan_image_pipeline_internal.hpp"),
                "using VulkanImageVertex = ImageVertex;") &&
                has(read("src/renderer/vulkan/vulkan_text_pipeline_internal.hpp"),
                    "using VulkanTextVertex = TextVertex;"),
            "Vulkan pipeline vertex types must be compatible aliases");
    const auto manifest = read("tests/renderer-neutral/mcpp.toml");
    require(!has(manifest, "renderer-vulkan") && !has(manifest, "renderer-dx12"),
            "neutral geometry tests cannot link a native renderer");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
