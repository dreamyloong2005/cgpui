#pragma once

#include "vulkan_backdrop_blur_recording_internal.hpp"
#include "vulkan_box_shadow_pipeline_internal.hpp"

namespace cgpui {

// Per-frame effect state handed to command recording. Both are empty when
// the frame has no shadow or backdrop blur.
struct VulkanFrameEffects {
  VulkanBoxShadowDrawResources box_shadow;
  VulkanBackdropBlurFrame backdrop_blur;
};

} // namespace cgpui
