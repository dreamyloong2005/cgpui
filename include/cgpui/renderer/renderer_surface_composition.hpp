#pragma once

#include <cstdint>

namespace cgpui {

enum class RendererCompositeAlpha {
  unknown,
  opaque,
  pre_multiplied,
  post_multiplied,
  inherit,
};

// How the renderer's presentation surface composites with what is behind
// the window. `supported_composite_alpha` is the raw
// VkSurfaceCapabilitiesKHR::supportedCompositeAlpha mask on Vulkan.
struct RendererSurfaceComposition {
  bool supported = false;
  bool transparent_requested = false;
  std::uint32_t supported_composite_alpha = 0;
  RendererCompositeAlpha composite_alpha = RendererCompositeAlpha::unknown;

  // Inherit leaves alpha to the native window, which CGPUI cannot confirm,
  // so only the explicit pre/post-multiplied modes count as composited.
  [[nodiscard]] constexpr bool alpha_composited() const {
    return composite_alpha == RendererCompositeAlpha::pre_multiplied ||
           composite_alpha == RendererCompositeAlpha::post_multiplied;
  }
};

} // namespace cgpui
