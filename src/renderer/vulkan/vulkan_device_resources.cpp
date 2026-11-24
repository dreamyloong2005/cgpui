#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::create_device_resources() {
  if (auto result = create_device(); !result) return result;
  if (auto result = vulkan_create_image_texture_descriptor_resources(
          device_, image_texture_resources_); !result) return result;
  if (auto result = create_command_pool(); !result) return result;
  if (auto result = vulkan_create_glyph_atlas_descriptor_resources(
          device_, glyph_atlas_resources_); !result) return result;
  if (auto result = create_swapchain(); !result) return result;
  return create_sync_objects();
}

void VulkanRendererState::destroy_device_resources(bool wait_idle) {
  if (device_ == VK_NULL_HANDLE) return;
  // A confirmed lost device cannot complete more work. Do not wait/retry it.
  if (wait_idle) (void)vkDeviceWaitIdle(device_);
  destroy_swapchain();
  composition_.reset(); // Imported image/transfer buffers precede the pool/device.
  destroy_sync_objects();
  vulkan_destroy_rounded_rect_buffers(device_, solid_rect_buffers_);
  vulkan_destroy_rounded_rect_buffers(device_, rounded_rect_buffers_);
  vulkan_destroy_rounded_rect_buffers(device_, box_shadow_buffers_);
  vulkan_destroy_backdrop_blur_resources(device_, backdrop_blur_resources_);
  vulkan_destroy_image_vertex_buffer(device_, image_vertex_buffer_);
  vulkan_destroy_text_vertex_buffer(device_, text_vertex_buffer_);
  vulkan_destroy_frame_pixel_capture(device_, frame_pixel_capture_);
  vulkan_destroy_image_texture_upload_resources(device_, image_texture_uploads_);
  vulkan_destroy_image_texture_resources(device_, image_texture_resources_);
  vulkan_destroy_image_texture_descriptor_resources(device_, image_texture_resources_);
  vulkan_destroy_glyph_atlas_upload_resources(device_, glyph_atlas_uploads_);
  vulkan_destroy_glyph_atlas_resources(device_, glyph_atlas_resources_);
  if (command_pool_ != VK_NULL_HANDLE) vkDestroyCommandPool(device_, command_pool_, nullptr);
  command_pool_ = VK_NULL_HANDLE;
  vkDestroyDevice(device_, nullptr);
  device_ = VK_NULL_HANDLE;
  graphics_queue_ = present_queue_ = VK_NULL_HANDLE;
  // Keep CPU GlyphCache/retained image payloads and latest successful pixels.
  // Empty GPU prefixes force full uploads on the replacement device.
  glyph_atlas_plan_state_ = {};
  pending_glyph_atlas_plan_state_ = {};
  glyph_atlas_draw_quads_.clear();
  glyph_atlas_draw_bindings_.clear();
  backdrop_blur_plan_ = {};
  pending_frame_upload_bytes_ = {};
}

} // namespace cgpui
