#include "vulkan_internal.hpp"

namespace cgpui {

// The shadow pipeline is created on the first frame with a shadow and lives
// with the swapchain render pass; frames without shadows upload nothing
// (empty geometry keeps the existing buffers).
Result<void> VulkanRendererState::prepare_box_shadow_frame(
    std::span<const BoxShadowDraw> box_shadows) {
  if (!box_shadows.empty() && !box_shadow_pipeline_resources_.ready()) {
    if (auto result = vulkan_create_box_shadow_pipeline_resources(
            device_, render_pass_, box_shadow_pipeline_resources_);
        !result) {
      return result;
    }
  }
  return vulkan_upload_box_shadow_buffers(
      physical_device_, device_, box_shadows, box_shadow_buffers_);
}

} // namespace cgpui
