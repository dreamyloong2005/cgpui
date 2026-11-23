#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

// C6 backdrop blur: planning, targets, pipelines, recording and frame
// preparation stay in focused modules; frames without a backdrop keep the
// direct swapchain pass and never create offscreen resources.
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
    limit("src/renderer/vulkan/vulkan_backdrop_blur_plan_internal.hpp", 70);
    limit("src/renderer/vulkan/vulkan_backdrop_blur_plan.cpp", 170);
    limit("include/cgpui/renderer/planning/backdrop_blur_vertex.hpp", 50);
    limit("include/cgpui/renderer/planning/backdrop_blur_pass.hpp", 50);
    limit("include/cgpui/renderer/planning/backdrop_blur_plan.hpp", 50);
    limit("include/cgpui/renderer/planning/backdrop_blur_parameters.hpp", 30);
    limit("src/renderer/planning/backdrop_blur_plan.cpp", 170);
    limit("src/renderer/planning/backdrop_blur_parameters.cpp", 30);
    limit("src/renderer/vulkan/vulkan_backdrop_blur_resources_internal.hpp", 95);
    limit("src/renderer/vulkan/vulkan_backdrop_blur_targets.cpp", 200);
    limit("src/renderer/vulkan/vulkan_backdrop_blur_descriptors.cpp", 100);
    limit("src/renderer/vulkan/vulkan_backdrop_blur_lifecycle.cpp", 90);
    limit("src/renderer/vulkan/vulkan_backdrop_blur_pipelines.cpp", 175);
    limit("src/renderer/vulkan/vulkan_backdrop_blur_recording_internal.hpp", 50);
    limit("src/renderer/vulkan/vulkan_backdrop_blur_recording.cpp", 210);
    limit("src/renderer/vulkan/vulkan_backdrop_blur_shader_binaries.cpp", 470);
    limit("src/renderer/vulkan/vulkan_frame_effects_internal.hpp", 25);
    limit("src/renderer/renderer_effect_fallback.cpp", 40);
    const auto effects = read("src/renderer/vulkan/vulkan_frame_effects.cpp");
    const auto plan = read("src/renderer/planning/backdrop_blur_plan.cpp");
    const auto adapter = read("src/renderer/vulkan/vulkan_backdrop_blur_plan.cpp");
    const auto recording = read("src/renderer/vulkan/vulkan_command_recording.cpp");
    const auto renderer = read("src/renderer/vulkan/vulkan_renderer.cpp");
    const auto fallback = read("src/renderer/renderer_effect_fallback.cpp");
    const auto blur = read("src/renderer/vulkan/shaders/backdrop.slang");
    const auto binaries =
        read("src/renderer/vulkan/vulkan_backdrop_blur_shader_binaries.cpp");
    const auto module = read("tests/renderer/mcpp.toml") +
      read("tests/ui/mcpp.toml");
    require(has(effects, "if (backdrops.empty()) {") &&
                has(effects, "vulkan_prepare_backdrop_blur_resources("),
            "offscreen resources are created only for frames with a backdrop");
    require(has(plan, "plan_backdrop_blur(") && has(plan, "blur_region(") &&
                has(plan, "resolve_rounded_rect_radii(") && !has(plan, "Vk"),
            "backdrop CPU regions and vertices have a native-free planning owner");
    require(has(adapter, "plan_backdrop_blur(") && has(adapter, "pass.region.x") &&
                has(adapter, "vulkan_backdrop_blur_support(") &&
                has(adapter, "vulkan_backdrop_blur_offscreen_pass_count(") &&
                !has(adapter, "std::ceil") && !has(adapter, "std::sqrt"),
            "Vulkan adapts plan types and retains its own capability and renderpass policy");
    require(has(recording, "if (effects.backdrop_blur.active()) {") &&
                has(recording, "vkCmdBeginRenderPass"),
            "the direct swapchain pass stays the path without backdrops");
    require(has(renderer, "backdrop_blur_support().supported") &&
                has(fallback, "draw_rounded_rect("),
            "without support the backdrop falls back to the tint only");
    require(has(blur, "push_constants.direction") && has(binaries, "generated/backdrop_blur_spirv.inc") &&
                has(read("src/renderer/vulkan/generated/backdrop_blur_spirv.inc"),
                    "scripts/dev/compile_shaders.py"),
            "the blur is separable and its SPIR-V records how it was generated");
    require(has(module, "\"vulkan_backdrop_blur_pixel_test.cpp\"") &&
                has(module, "\"vulkan_backdrop_blur_plan_test.cpp\"") &&
                has(module, "\"backdrop_blur_submission_test.cpp\""),
            "the behavior tests are owned by the capability module");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "backdrop blur structure failed: " << error.what() << '\n';
    return 1;
  }
}
