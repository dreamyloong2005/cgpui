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
  if (!file) throw std::runtime_error("missing backdrop owner: " + name);
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
    for (const char* leaf : {"backdrop_blur_vertex", "backdrop_blur_pass",
                             "backdrop_blur_plan", "backdrop_blur_parameters"}) {
      const auto source = read(std::string("include/cgpui/renderer/planning/") + leaf + ".hpp");
      require(std::count(source.begin(), source.end(), '\n') <= 50 &&
                  !has(source, "vulkan") && !has(source, "Vk") &&
                  !has(source, "windows.h") && !has(source, "src/") &&
                  !has(source, "offscreen_pass_count") && !has(source, "for ("),
              "backdrop public leaves must be native-free planning declarations");
    }
    const auto planner = read("src/renderer/planning/backdrop_blur_plan.cpp");
    require(std::count(planner.begin(), planner.end(), '\n') <= 170 &&
                has(planner, "PixelExtent") && has(planner, "PixelScissor") &&
                has(planner, "resolve_rounded_rect_radii(") &&
                has(planner, "resolve_effective_clip_rect(") &&
                !has(planner, "vulkan_") && !has(planner, "Vk"),
            "one neutral owner must implement region/vertex/pass planning");
    const auto adapter = read("src/renderer/vulkan/vulkan_backdrop_blur_plan.cpp");
    require(has(adapter, "plan_backdrop_blur(") && has(adapter, "pass.region.x") &&
                has(adapter, "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT") &&
                has(adapter, "vulkan_backdrop_blur_offscreen_pass_count(") &&
                !has(adapter, "std::ceil") && !has(adapter, "std::sqrt") &&
                !has(adapter, "resolve_rounded_rect_radii("),
            "Vulkan retains native capability/schedule policy but only adapts shared plans");
    require(has(read("src/renderer/vulkan/vulkan_backdrop_blur_plan_internal.hpp"),
                "using VulkanBackdropBlurVertex = BackdropBlurVertex;"),
            "Vulkan composite vertices must alias the shared public layout");
    const auto parameters = read("src/renderer/planning/backdrop_blur_parameters.cpp");
    require(has(parameters, "backdrop_blur_downsample_factor(") &&
                !has(parameters, "offscreen_pass_count") && !has(parameters, "vulkan"),
            "shared sampling policy must not own a native renderpass count");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
