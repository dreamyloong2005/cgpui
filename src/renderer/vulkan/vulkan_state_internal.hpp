#pragma once

#include "vulkan_device_internal.hpp"
#include "vulkan_frame_draw_order_internal.hpp"
#include "vulkan_glyph_atlas_draw_bindings_internal.hpp"
#include "vulkan_glyph_atlas_resources_internal.hpp"
#include "vulkan_glyph_atlas_uploads_internal.hpp"
#include "vulkan_image_texture_resources_internal.hpp"
#include "vulkan_rounded_rect_buffers_internal.hpp"
#include "vulkan_swapchain_internal.hpp"
#include "vulkan_text_vertex_buffer_internal.hpp"

namespace cgpui {

class VulkanRendererState final {
 public:
  ~VulkanRendererState();

  static Result<std::shared_ptr<VulkanRendererState>> create(
      RenderSurfaceDescriptor descriptor);

  Result<void> resize(Size framebuffer_size, DpiScale scale);
  Result<void> present_frame(
      Color color,
      std::span<const VulkanFrameDrawOrderEntry> draw_order,
      std::span<const SolidRect> rects,
      std::span<const RoundedRectDraw> rounded_rects,
      std::span<const TextDraw> text_draws,
      std::span<const TextSelectionDraw> text_selections,
      std::span<const TextCaretDraw> text_carets,
      std::span<const ImageDraw> image_draws);

 private:
  explicit VulkanRendererState(RenderSurfaceDescriptor descriptor);

  void destroy_swapchain_resources(VulkanSwapchainResources& resources);
  void destroy_sync_objects();
  void destroy_swapchain();
  void install_swapchain(VulkanSwapchainResources&& resources);

  Result<void> create_command_pool();
  Result<void> create_sync_objects();
  Result<void> prepare_glyph_atlas_frame(
      std::span<const TextDraw> text_draws);
  Result<void> prepare_solid_rect_frame(
      std::span<const SolidRect> rects);
  Result<void> prepare_rounded_rect_frame(
      std::span<const RoundedRectDraw> rounded_rects);
  Result<void> prepare_image_texture_frame(
      std::span<const ImageDraw> image_draws);
  void commit_glyph_atlas_frame();
  Result<void> recover_after_failed_submit(std::string message);
  Result<void> recover_after_failed_record(std::string message);
  Result<void> recover_after_failed_acquire(
      ErrorCode code,
      std::string message,
      bool block_presentation);
  Result<void> recover_after_failed_present(std::string message);

  Result<void> create_swapchain();
  Result<VulkanSwapchainResources> create_swapchain_resources(
      VkSwapchainKHR old_swapchain,
      bool* retired_old_swapchain = nullptr);
  Result<void> populate_swapchain_images(
      VulkanSwapchainResources& resources);
  Result<void> create_swapchain_image_views(
      VulkanSwapchainResources& resources);
  Result<void> create_swapchain_render_pass(
      VulkanSwapchainResources& resources);
  Result<void> create_swapchain_framebuffers(
      VulkanSwapchainResources& resources);
  Result<void> allocate_swapchain_command_buffers(
      VulkanSwapchainResources& resources);

  Result<void> create_instance();
  Result<void> create_surface();
  Result<VulkanQueueFamilySearch> find_queue_families(
      VkPhysicalDevice device) const;
  Result<bool> has_device_extension(
      VkPhysicalDevice device,
      const char* extension_name) const;
  Result<void> select_physical_device();
  Result<void> create_device();

  RenderSurfaceDescriptor descriptor_;
  VkInstance instance_ = VK_NULL_HANDLE;
  VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  std::uint32_t graphics_queue_family_ = 0;
  std::uint32_t present_queue_family_ = 0;
  VkQueue graphics_queue_ = VK_NULL_HANDLE;
  VkQueue present_queue_ = VK_NULL_HANDLE;
  VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
  VkFormat swapchain_format_ = VK_FORMAT_UNDEFINED;
  VkExtent2D swapchain_extent_{};
  std::vector<VkImage> swapchain_images_;
  std::vector<VkImageView> swapchain_image_views_;
  VkRenderPass render_pass_ = VK_NULL_HANDLE;
  VulkanTextPipelineResources text_pipeline_resources_;
  VulkanRoundedRectPipelineResources rounded_rect_pipeline_resources_;
  std::vector<VkFramebuffer> framebuffers_;
  VkCommandPool command_pool_ = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> command_buffers_;
  VkSemaphore image_available_ = VK_NULL_HANDLE;
  VkSemaphore render_finished_ = VK_NULL_HANDLE;
  VkFence in_flight_ = VK_NULL_HANDLE;
  GlyphCache glyph_cache_;
  GlyphAtlasProductionResourceState glyph_atlas_plan_state_;
  GlyphAtlasProductionResourceState pending_glyph_atlas_plan_state_;
  VulkanGlyphAtlasResources glyph_atlas_resources_;
  VulkanGlyphAtlasUploadResources glyph_atlas_uploads_;
  VulkanImageTextureResources image_texture_resources_;
  std::vector<TexturedGlyphQuad> glyph_atlas_draw_quads_;
  std::vector<VulkanGlyphAtlasDrawBinding> glyph_atlas_draw_bindings_;
  VulkanTextVertexBufferResources text_vertex_buffer_;
  VulkanRoundedRectBufferResources solid_rect_buffers_;
  VulkanRoundedRectBufferResources rounded_rect_buffers_;
  std::vector<RendererCommandBatch> last_command_batches_;
  bool presentation_blocked_ = false;
};

} // namespace cgpui
