#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

// C5 box shadow: neutral CPU planning and focused Vulkan GPU modules, with
// line limits and the UI submitting shadows to the renderer.
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
void limit(const char* path, std::size_t max_lines) {
  if (lines(read(path)) > max_lines) {
    throw std::runtime_error(std::string("module over its line limit: ") + path);
  }
}
} // namespace

int main() {
  try {
    limit("include/cgpui/renderer/renderer_effect_draws.hpp", 70);
    limit("src/renderer/vulkan/vulkan_box_shadow_geometry_internal.hpp", 50);
    limit("src/renderer/vulkan/vulkan_box_shadow_geometry.cpp", 140);
    limit("include/cgpui/renderer/planning/box_shadow_vertex.hpp", 50);
    limit("include/cgpui/renderer/planning/box_shadow_geometry.hpp", 50);
    limit("include/cgpui/renderer/planning/box_shadow_parameters.hpp", 30);
    limit("src/renderer/planning/box_shadow_geometry.cpp", 140);
    limit("src/renderer/planning/box_shadow_parameters.cpp", 40);
    limit("src/renderer/vulkan/vulkan_box_shadow_pipeline_internal.hpp", 45);
    limit("src/renderer/vulkan/vulkan_box_shadow_pipeline.cpp", 140);
    limit("src/renderer/vulkan/vulkan_box_shadow_buffers.cpp", 90);
    limit("src/renderer/vulkan/vulkan_frame_effects.cpp", 90);
    limit("src/renderer/vulkan/vulkan_box_shadow_shader_binaries.cpp", 360);
    const auto effects = read("include/cgpui/renderer/renderer_effect_draws.hpp");
    const auto fragment = read("src/renderer/vulkan/shaders/box_shadow.slang");
    const auto binaries =
        read("src/renderer/vulkan/vulkan_box_shadow_shader_binaries.cpp");
    const auto geometry = read("src/renderer/planning/box_shadow_geometry.cpp");
    const auto parameters = read("src/renderer/planning/box_shadow_parameters.cpp");
    const auto submit = read("src/ui/render_view_commands.cpp");
    const auto recording =
        read("src/renderer/vulkan/vulkan_frame_draw_recording.cpp");
    const auto module = read("tests/renderer/mcpp.toml") +
      read("tests/ui/mcpp.toml");
    require(has(effects, "struct BoxShadowDraw") && !has(effects, "vulkan"),
            "the shadow draw is a backend-neutral public leaf");
    require(has(fragment, "erf_approx(") && has(fragment, "1.0 - box") &&
                has(fragment, "output.local = input.local"),
            "the analytic erf shadow and its CSS clip-out live in the shader");
    require(has(binaries, "generated/box_shadow_spirv.inc") &&
                has(read("src/renderer/vulkan/generated/box_shadow_spirv.inc"),
                    "0x07230203U"),
            "embedded SPIR-V records how it was generated");
    require(has(parameters, "blur_radius) * 0.5F") &&
                has(geometry, "apply_composed_transform(") &&
                has(geometry, "apply_composed_opacity("),
            "sigma = blur / 2, with transform and opacity metadata applied");
    require(has(submit, "frame.draw_box_shadow(") &&
                has(recording, "VulkanFrameDrawResourceKind::box_shadow"),
            "shadow commands reach the frame and keep their draw order");
    require(has(module, "\"vulkan_box_shadow_pixel_test.cpp\"") &&
                has(module, "\"vulkan_box_shadow_geometry_test.cpp\"") &&
                has(module, "\"box_shadow_submission_test.cpp\""),
            "the behavior tests are owned by the capability module");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "box shadow structure failed: " << error.what() << '\n';
    return 1;
  }
}
