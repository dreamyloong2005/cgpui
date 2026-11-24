#include "vulkan_missing_image_native_reuse_test_support.hpp"
#include "renderer_pixel_test_support.hpp"
#include "cgpui/renderer/renderer_backend.hpp"
#include <cstdio>

using namespace cgpui;
using namespace cgpui_test;
int main() {
  RendererPixelTestWindow window(L"CGPUIVulkanMissingNativeReuse", L"CGPUI Vulkan Missing Native Reuse");
  if (!window.valid()) return 1;
  auto renderer = create_vulkan_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}, .transparent_background = false});
  if (!renderer || !*renderer) return 2;
  if (!validate_pixel_backend_info(RendererBackend::vulkan, (*renderer)->backend_info())) return 3;
  VulkanNativeSubmissionTrace trace; // Factory admitted actual loader; forwards only native functions.
  trace.begin_frame(0);
  auto seed_frame = begin_pixel_capture(**renderer);
  if (!seed_frame) return 4;
  (*seed_frame)->clear({0, 0, 0, 1});
  if (!(*seed_frame)->present()) return 5;
  const auto seed = completed_pixel_capture(**renderer); // Owned successful historical capture.
  if (!seed || !pixel_near(*seed, 12, 32, {0, 0, 0, 255}) ||
      !pixel_near(*seed, 56, 56, {0, 0, 0, 255})) return 6;
  const auto images = missing_reuse_images();
  const std::array phases{
      VulkanMissingReusePhase{1, "both-missing", {0, 1, 1, 1}, 0, 0, 0},
      VulkanMissingReusePhase{2, "first-uploaded", {1, 0, 1, 1}, 1, 1, 0},
      VulkanMissingReusePhase{3, "both-ready", {0, 0, 1, 1}, 3, 2, 0},
      VulkanMissingReusePhase{4, "second-invalidated", {1, 0, 0, 1}, 1, 0, 2},
      VulkanMissingReusePhase{5, "both-invalidated", {0, 1, 0, 1}, 0, 0, 1},
      VulkanMissingReusePhase{6, "second-resupplied", {1, 1, 0, 1}, 2, 2, 0},
  };
  for (const auto& phase : phases) {
    if (const int result = run_missing_reuse_phase(**renderer, trace, images, *seed, phase)) {
      std::fprintf(stderr, "native reuse phase=%s failed=%d\n", phase.name, result); return result;
    }
  }
  trace.begin_frame(7);
  auto final_frame = begin_pixel_capture(**renderer); // Real readback intentionally forces fresh recording.
  if (!final_frame) return 7;
  (*final_frame)->clear({1, 1, 0, 1}); draw_missing_reuse_scene(**final_frame, images);
  if (!(*final_frame)->present()) return 8;
  const auto final = completed_pixel_capture(**renderer);
  const auto observed = trace.observation();
  if (!observed.reliable || observed.reused || observed.begin_count != 1 ||
      observed.successful_submit_count != 1 || observed.submitted_buffer_count != 1 ||
      observed.recorded_phase != 7 || observed.indexed_draw_count != 2 || observed.direct_draw_count != 1 ||
      !vulkan_observed_missing_scene_matches(observed.sequence, 2) ||
      !missing_reuse_diagnostics((*renderer)->last_frame_diagnostic_snapshot(), 2, 0)) return 9;
  if (!final || final->rgba8 == seed->rgba8 || !pixel_near(*final, 12, 12, {255, 0, 0, 255}) ||
      !pixel_near(*final, 12, 32, {255, 255, 0, 255}) || !pixel_near(*final, 36, 12, {255, 0, 255, 255}) ||
      !pixel_near(*final, 44, 12, {0, 0, 255, 255}) || !pixel_near(*final, 36, 32, {255, 0, 255, 255}) ||
      !pixel_near(*final, 56, 56, {255, 255, 0, 255})) return 10;
  std::puts("VULKAN_NATIVE_REUSE_SUCCESS phases=6 fresh-each=yes reused-each>=2 history-only-during-loop=yes final-real-capture=yes");
  // The Windows ASan entry terminates the process without ordinary CRT exit.
  if (std::fflush(stdout) != 0 || std::ferror(stdout)) return 27;
  return 0;
}
