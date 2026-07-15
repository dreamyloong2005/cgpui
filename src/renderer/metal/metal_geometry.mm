#include "metal_renderer_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

void bind_vertices(
    id<MTLRenderCommandEncoder> encoder,
    const std::array<MetalVertex, 4>& vertices,
    Size framebuffer_size) {
  const simd_float2 viewport{
      framebuffer_size.width, framebuffer_size.height};
  [encoder setVertexBytes:vertices.data()
                  length:sizeof(vertices)
                 atIndex:0];
  [encoder setVertexBytes:&viewport length:sizeof(viewport) atIndex:1];
}

Result<void> apply_clip(
    id<MTLRenderCommandEncoder> encoder,
    MetalRendererState& state,
    std::optional<Rect> clip,
    const RendererClipStackRecord& stack) {
  const MetalClipResolution resolved =
      metal_resolve_clip(state.framebuffer_size, clip, stack);
  if (!resolved.visible) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "Metal draw is fully clipped"});
  }
  [encoder setScissorRect:resolved.scissor];
  return {};
}

}  // namespace

Result<void> metal_encode_solid_rect(
    id<MTLRenderCommandEncoder> encoder,
    MetalRendererState& state,
    const SolidRect& rect) {
  if (rect.rect.size.width <= 0.0F || rect.rect.size.height <= 0.0F) return {};
  auto clip = apply_clip(encoder, state, rect.clip_rect, rect.clip_stack);
  if (!clip) return {};
  const auto vertices = metal_rect_vertices(
      rect.rect, rect.color, rect.metadata, rect.composition_stack);
  [encoder setRenderPipelineState:state.pipelines.solid];
  bind_vertices(encoder, vertices, state.framebuffer_size);
  [encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip
              vertexStart:0
              vertexCount:4];
  return {};
}

Result<void> metal_encode_rounded_rect(
    id<MTLRenderCommandEncoder> encoder,
    MetalRendererState& state,
    const RoundedRectDraw& rect) {
  if (rect.rect.size.width <= 0.0F || rect.rect.size.height <= 0.0F) return {};
  auto clip = apply_clip(encoder, state, rect.clip_rect, rect.clip_stack);
  if (!clip) return {};
  const auto vertices = metal_rect_vertices(
      rect.rect, rect.color, rect.metadata, rect.composition_stack);
  const float opacity = vertices.front().color.w /
                        std::max(rect.color.a, 0.0001F);
  const Color border = rect.border_color.value_or(rect.color);
  const MetalRoundedRectUniforms uniforms{
      .fill_color = vertices.front().color,
      .border_color = {border.r, border.g, border.b, border.a * opacity},
      .radii = {rect.radius.top_left,
                rect.radius.top_right,
                rect.radius.bottom_right,
                rect.radius.bottom_left},
      .size = {rect.rect.size.width, rect.rect.size.height},
      .border_width = std::max(0.0F, rect.border_width),
      .fill_enabled = rect.fill_enabled ? 1.0F : 0.0F};
  [encoder setRenderPipelineState:state.pipelines.rounded];
  bind_vertices(encoder, vertices, state.framebuffer_size);
  [encoder setFragmentBytes:&uniforms length:sizeof(uniforms) atIndex:0];
  [encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip
              vertexStart:0
              vertexCount:4];
  return {};
}

}  // namespace cgpui
