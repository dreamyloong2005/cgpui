#include "vulkan_box_shadow_pipeline_internal.hpp"

namespace cgpui {
namespace {

// SPIR-V compiled from Slang by scripts/dev/compile_shaders.py.
#include "generated/box_shadow_spirv.inc"

} // namespace

std::span<const std::uint32_t> vulkan_box_shadow_vertex_shader_spirv() {
  return box_shadow_vertex_shader_spirv;
}

std::span<const std::uint32_t> vulkan_box_shadow_fragment_shader_spirv() {
  return box_shadow_fragment_shader_spirv;
}

} // namespace cgpui
