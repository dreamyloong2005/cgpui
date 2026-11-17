#include "vulkan_surface_composition_internal.hpp"

#include <iostream>

namespace {

using cgpui::RendererCompositeAlpha;

int fail(int code, const char* message) {
  std::cerr << "vulkan surface composition: " << message << '\n';
  return code;
}

bool reports(VkCompositeAlphaFlagsKHR mask, bool transparent,
             RendererCompositeAlpha expected, bool composited) {
  const cgpui::RendererSurfaceComposition composition =
      cgpui::vulkan_surface_composition(mask, transparent);
  return composition.supported &&
         composition.transparent_requested == transparent &&
         composition.supported_composite_alpha == mask &&
         composition.composite_alpha == expected &&
         composition.alpha_composited() == composited;
}

} // namespace

int main() {
  constexpr VkCompositeAlphaFlagsKHR opaque = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  constexpr VkCompositeAlphaFlagsKHR pre = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
  constexpr VkCompositeAlphaFlagsKHR post = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
  constexpr VkCompositeAlphaFlagsKHR inherit = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
  // Masks observed on this machine: NVIDIA Win32 and every WSLg Wayland
  // device report opaque | pre; Intel Win32 reports opaque | inherit.
  if (!reports(opaque | pre, true, RendererCompositeAlpha::pre_multiplied, true)) {
    return fail(1, "pre-multiplied must win for a transparent request");
  }
  if (!reports(opaque | inherit, true, RendererCompositeAlpha::inherit, false)) {
    return fail(2, "inherit is chosen but not reported as composited");
  }
  if (!reports(opaque | pre | inherit, true,
               RendererCompositeAlpha::pre_multiplied, true)) {
    return fail(3, "pre-multiplied must be preferred over inherit");
  }
  if (!reports(opaque | post, true, RendererCompositeAlpha::opaque, false) ||
      !reports(opaque, true, RendererCompositeAlpha::opaque, false)) {
    return fail(4, "without pre or inherit the surface stays opaque");
  }
  if (!reports(opaque | pre, false, RendererCompositeAlpha::opaque, false)) {
    return fail(5, "an opaque request must stay opaque");
  }
  if (!reports(pre, false, RendererCompositeAlpha::pre_multiplied, true)) {
    return fail(6, "a surface without opaque falls through the preference");
  }
  if (cgpui::vulkan_renderer_composite_alpha(
          static_cast<VkCompositeAlphaFlagBitsKHR>(0x40)) !=
      RendererCompositeAlpha::unknown) {
    return fail(7, "unknown bits must map to unknown");
  }
  return 0;
}
