#pragma once

// Box shadow (metal_box_shadow.mm) and backdrop blur (metal_backdrop_blur.mm).
#include "metal_renderer_internal.hpp"

namespace cgpui {

// One box shadow quad (metal_box_shadow.mm). Rects are {min x, min y,
// max x, max y}; radii {top left, top right, bottom right, bottom left}.
struct MetalBoxShadowVertex {
  simd_float2 position{};
  simd_float2 local{};
  simd_float4 color{};
};

struct MetalBoxShadowUniforms {
  simd_float4 shadow_rect{};
  simd_float4 shadow_radii{};
  simd_float4 box_rect{};
  simd_float4 box_radii{};
  float sigma = 0.0F;
};

struct MetalBoxShadowQuad {
  std::array<MetalBoxShadowVertex, 4> vertices{};
  MetalBoxShadowUniforms uniforms{};
};

// CSS semantics as on Vulkan: sigma = blur / 2, spread grows the shape and
// its radii (css-backgrounds-3 7.1.1), the quad covers the shape plus
// 3 sigma + 1 px. nullopt when nothing would be drawn.
[[nodiscard]] std::optional<MetalBoxShadowQuad> metal_box_shadow_quad(
    const BoxShadowDraw& shadow);

struct MetalBackdropUniforms {
  simd_float4 rect{};
  simd_float4 radii{};
  simd_float4 tint{};
};

// Scene and blur textures of the drawable's size.
Result<void> metal_ensure_backdrop_targets(
    MetalRendererState& state,
    NSUInteger width,
    NSUInteger height);
// Ends `encoder`, blurs the backdrop region of the scene with
// MPSImageGaussianBlur, and returns a new encoder on the scene that has
// drawn the blurred region with the rounded mask and the tint.
Result<id<MTLRenderCommandEncoder>> metal_encode_backdrop_blur(
    id<MTLCommandBuffer> command_buffer,
    id<MTLRenderCommandEncoder> encoder,
    MetalRendererState& state,
    const BackdropBlurDraw& backdrop);
// Copies the scene into the drawable (no blending).
Result<void> metal_encode_scene_copy(
    id<MTLCommandBuffer> command_buffer,
    MetalRendererState& state,
    id<MTLTexture> drawable);

Result<std::size_t> metal_encode_frame_draw(
    id<MTLCommandBuffer> command_buffer,
    __strong id<MTLRenderCommandEncoder>& encoder,
    MetalRendererState& state,
    const MetalFrameCommands& commands,
    MetalFrameDrawOrderEntry entry);

}  // namespace cgpui
