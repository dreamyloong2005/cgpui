#include "vulkan_missing_image_native_reuse_test_support.hpp"
#include "renderer_pixel_test_backend.hpp"
#include "../support/frame_wait_test_support.hpp"
#include <cstdio>

namespace cgpui_test {
int run_missing_reuse_phase(cgpui::Renderer& renderer, VulkanNativeSubmissionTrace& trace,
    const std::array<cgpui::ImageAsset, 2>& images, const cgpui::RendererFramePixels& seed,
    const VulkanMissingReusePhase& phase) {
  unsigned fresh = 0, reused = 0;
  const auto generation = renderer.backend_info().device_generation;
  // Bound independent frame attempts; a Present error never retries/advances.
  for (unsigned attempt = 0; attempt != 24; ++attempt) {
    trace.begin_frame(phase.number);
    auto frame = wait_for_test_frame(renderer);
    if (!frame) { report_pixel_test_error("native reuse begin", frame.error()); return 20; }
    (*frame)->clear(phase.clear);
    for (unsigned index = 0; index != 2 && attempt == 0; ++index) {
      if (phase.invalidate_mask & (1U << index)) (*frame)->invalidate_image(images[index].id);
      if (phase.upload_mask & (1U << index)) (*frame)->upload_image(images[index]);
    }
    draw_missing_reuse_scene(**frame, images); // Deliberately NO capture request in this loop.
    if (auto result = (*frame)->present(); !result) {
      report_pixel_test_error("native reuse Present (not retried)", result.error()); return 21;
    }
    const auto observed = trace.observation();
    const unsigned ready = unsigned(bool(phase.available_mask & 1)) + unsigned(bool(phase.available_mask & 2));
    const unsigned upload_bytes = attempt ? 0 : 4 *
        (unsigned(bool(phase.upload_mask & 1)) + unsigned(bool(phase.upload_mask & 2)));
    if (!observed.reliable || observed.successful_submit_count != 1 || observed.submitted_buffer_count != 1 ||
        !observed.buffer || !observed.recording_epoch || observed.recorded_phase != phase.number ||
        observed.indexed_draw_count != 2 || observed.direct_draw_count != ready || (observed.begin_count == 0) != observed.reused ||
        observed.begin_count > 1 || (attempt == 0 && (observed.reused || observed.begin_count != 1))) return 22;
    if (!vulkan_observed_missing_scene_matches(observed.sequence, phase.available_mask)) return 26;
    if (!missing_reuse_diagnostics(renderer.last_frame_diagnostic_snapshot(), phase.available_mask, upload_bytes)) return 23;
    // This is history preservation only, never proof of the current uncaptured pixels.
    if (!same_missing_reuse_capture(renderer.last_frame_pixels(), seed) ||
        renderer.backend_info().device_generation != generation) return 24;
    fresh += !observed.reused; reused += observed.reused;
    std::printf("VULKAN_NATIVE_REUSE phase=%s attempt=%u buffer=%p epoch=%llu begin=%u reused=%u direct=%u missing-mask=%u\n",
        phase.name, attempt, static_cast<void*>(observed.buffer),
        static_cast<unsigned long long>(observed.recording_epoch), observed.begin_count,
        unsigned(observed.reused), observed.direct_draw_count, (~phase.available_mask) & 3U);
    std::fputs("VULKAN_NATIVE_ORDER", stdout);
    for (unsigned index = 0; index != observed.sequence.size; ++index) {
      const auto& draw = observed.sequence.draws[index];
      std::printf(" %c%u", draw.kind == VulkanObservedDrawKind::direct ? 'D' : 'I', draw.first / 6);
    }
    std::fputc('\n', stdout);
    if (fresh && reused >= 2) return 0; // Require actual repeated native buffer submissions, not a prediction.
  }
  std::fprintf(stderr, "VULKAN_NATIVE_REUSE_NOT_OBSERVED phase=%s fresh=%u reused=%u\n", phase.name, fresh, reused);
  return 25; // A missing observation fails; never turn it into skip/pass.
}
} // namespace cgpui_test
