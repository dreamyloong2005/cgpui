#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read(const char* path) {
  std::ifstream stream(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  return {std::istreambuf_iterator<char>(stream), {}};
}
bool has(const std::string& s, const char* text) { return s.find(text) != std::string::npos; }
}
int main() {
  const auto observer = read("tests/renderer/vulkan_missing_image_native_trace_test_support.cpp");
  const auto loop = read("tests/renderer/vulkan_missing_image_native_reuse_loop_test_support.cpp");
  const auto main = read("tests/renderer/vulkan_missing_image_native_reuse_test.cpp");
  const auto manifest = read("tests/renderer/mcpp.toml");
  const auto sequence = read("tests/renderer/vulkan_missing_image_native_draw_sequence_test_support.cpp");
  if (!has(observer, "const VkResult result = trace.submit(queue, count, submits, fence)") ||
      !has(observer, "if (result != VK_SUCCESS) return result") ||
      !has(observer, "record->indexed_draws == 2") || !has(observer, "record->begun_frame != trace.frame") ||
      !has(observer, "o.sequence = record->sequence") || !has(observer, "record->sequence = {}") ||
      !has(observer, "{VulkanObservedDrawKind::direct, vertices, instances, first, 0, first_instance}") ||
      !has(observer, "{VulkanObservedDrawKind::indexed, count, instances, first, offset, first_instance}") ||
      !has(observer, "record->submissions != 0") || !has(observer, "++trace.epoch") ||
      !has(observer, "record->submissions = 0") || !has(observer, "*record = {}") ||
      !has(observer, "cgpui::vkQueueSubmit = trace.submit") || !has(observer, "cgpui::vkCmdDraw = trace.draw") ||
      !has(observer, "cgpui::vkResetCommandBuffer = trace.reset") ||
      has(observer, "return VK_SUCCESS") || has(observer, "std::vector<")) return 1;
  if (has(loop, "request_pixel_capture(") || has(loop, "begin_pixel_capture(") ||
      !has(loop, "attempt != 24") || !has(loop, "reused >= 2") ||
      !has(sequence, "actual.first % 6 != 0") || !has(sequence, "actual != expected.draws[index]") ||
      !has(loop, "vulkan_observed_missing_scene_matches(observed.sequence, phase.available_mask)") ||
      !has(main, "vulkan_observed_missing_scene_matches(observed.sequence, 2)") ||
      !has(loop, "observed.recorded_phase != phase.number") ||
      !has(loop, "same_missing_reuse_capture(renderer.last_frame_pixels(), seed)") ||
      !has(loop, "VULKAN_NATIVE_REUSE_NOT_OBSERVED") ||
      !has(main, "create_vulkan_renderer(") || !has(main, "validate_pixel_backend_info(RendererBackend::vulkan") ||
      !has(main, "final->rgba8 == seed->rgba8")) return 2;
  for (const auto* name : {"vulkan_missing_image_native_trace_test_support.cpp",
      "vulkan_missing_image_native_reuse_scene_test_support.cpp",
      "vulkan_missing_image_native_reuse_loop_test_support.cpp", "vulkan_missing_image_native_reuse_test.cpp",
      "vulkan_missing_image_native_draw_sequence_test_support.cpp", "vulkan_missing_image_native_draw_sequence_test.cpp"})
    if (!has(manifest, name)) return 3;
  if (!has(read("tests/architecture/mcpp.toml"), "vulkan_missing_image_native_reuse_structure_test.cpp") ||
      !has(read("scripts/ci/architecture-header-targets.txt"), "vulkan_missing_image_native_reuse_structure_test|")) return 4;
  return std::count(observer.begin(), observer.end(), '\n') <= 150 &&
      std::count(loop.begin(), loop.end(), '\n') <= 90 &&
      std::count(main.begin(), main.end(), '\n') <= 100 ? 0 : 5;
}
