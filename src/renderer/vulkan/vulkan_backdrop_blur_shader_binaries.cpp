#include "vulkan_backdrop_blur_resources_internal.hpp"

namespace cgpui {
namespace {

// SPIR-V compiled from Slang by scripts/dev/compile_shaders.py.
#include "generated/backdrop_blur_spirv.inc"

} // namespace

std::span<const std::uint32_t> vulkan_backdrop_blur_shader_spirv(
    std::size_t index) {
  switch (index) {
    case 0:
      return backdrop_fullscreen_vert_spirv;
    case 1:
      return backdrop_downsample_frag_spirv;
    case 2:
      return backdrop_blur_frag_spirv;
    case 3:
      return backdrop_present_frag_spirv;
    case 4:
      return backdrop_composite_vert_spirv;
    default:
      return backdrop_composite_frag_spirv;
  }
}

} // namespace cgpui
