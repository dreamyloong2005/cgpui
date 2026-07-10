#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::prepare_image_texture_frame(
    std::span<const ImageDraw> image_draws) {
  return vulkan_update_image_texture_resources(
      physical_device_,
      device_,
      image_draws,
      image_texture_resources_);
}

} // namespace cgpui
