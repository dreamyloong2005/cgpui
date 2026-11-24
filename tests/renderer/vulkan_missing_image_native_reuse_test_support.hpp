#pragma once
#include "vulkan_missing_image_native_trace_test_support.hpp"
#include <array>

namespace cgpui_test {
struct VulkanMissingReusePhase {
  unsigned number = 0;
  const char* name = "";
  cgpui::Color clear;
  unsigned available_mask = 0, upload_mask = 0, invalidate_mask = 0;
};
std::array<cgpui::ImageAsset, 2> missing_reuse_images();
void draw_missing_reuse_scene(cgpui::RenderFrame& frame,
    const std::array<cgpui::ImageAsset, 2>& images);
bool same_missing_reuse_capture(const cgpui::RendererFramePixels* actual,
    const cgpui::RendererFramePixels& seed);
bool missing_reuse_diagnostics(const cgpui::RendererFrameDiagnosticSnapshot* snapshot,
    unsigned available_mask, unsigned upload_bytes);
int run_missing_reuse_phase(cgpui::Renderer& renderer, VulkanNativeSubmissionTrace& trace,
    const std::array<cgpui::ImageAsset, 2>& images, const cgpui::RendererFramePixels& seed,
    const VulkanMissingReusePhase& phase);
} // namespace cgpui_test
