#include "metal_effects_internal.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {
namespace {

float fit_scale(float length, float sum) {
  return sum > 0.0F ? length / sum : 1.0F;
}

// CSS corner fitting: radii that do not fit are scaled down together.
BorderRadii fit_radii(Size size, BorderRadii requested) {
  if (size.width <= 0.0F || size.height <= 0.0F) return {};
  const BorderRadii r = BorderRadii::corners(
      std::max(0.0F, requested.top_left), std::max(0.0F, requested.top_right),
      std::max(0.0F, requested.bottom_right), std::max(0.0F, requested.bottom_left));
  const float scale = std::clamp(
      std::min({1.0F, fit_scale(size.width, r.top_left + r.top_right),
                fit_scale(size.width, r.bottom_left + r.bottom_right),
                fit_scale(size.height, r.top_left + r.bottom_left),
                fit_scale(size.height, r.top_right + r.bottom_right)}),
      0.0F, 1.0F);
  return BorderRadii::corners(r.top_left * scale, r.top_right * scale,
                              r.bottom_right * scale, r.bottom_left * scale);
}

float spread_radius(float radius, float spread) {
  if (radius <= 0.0F) return 0.0F;
  if (spread <= 0.0F || radius >= spread) return std::max(0.0F, radius + spread);
  const float ratio = radius / spread - 1.0F;
  return radius + spread * (1.0F + ratio * ratio * ratio);
}

simd_float4 bounds(Rect rect) {
  return {rect.origin.x, rect.origin.y, rect.origin.x + rect.size.width,
          rect.origin.y + rect.size.height};
}

simd_float4 corners(BorderRadii radii) {
  return {radii.top_left, radii.top_right, radii.bottom_right, radii.bottom_left};
}

}  // namespace

std::optional<MetalBoxShadowQuad> metal_box_shadow_quad(
    const BoxShadowDraw& draw) {
  const float spread =
      std::isfinite(draw.shadow.spread_radius) ? draw.shadow.spread_radius : 0.0F;
  const Rect shape{
      .origin = {draw.rect.origin.x + draw.shadow.offset.x - spread,
                 draw.rect.origin.y + draw.shadow.offset.y - spread},
      .size = {draw.rect.size.width + spread * 2.0F,
               draw.rect.size.height + spread * 2.0F}};
  if (draw.rect.size.width <= 0.0F || draw.rect.size.height <= 0.0F ||
      shape.size.width <= 0.0F || shape.size.height <= 0.0F) {
    return std::nullopt;
  }
  const BorderRadii box_radii = fit_radii(draw.rect.size, draw.radius);
  const BorderRadii shape_radii = fit_radii(
      shape.size,
      BorderRadii::corners(spread_radius(box_radii.top_left, spread),
                           spread_radius(box_radii.top_right, spread),
                           spread_radius(box_radii.bottom_right, spread),
                           spread_radius(box_radii.bottom_left, spread)));
  const float sigma = std::isfinite(draw.shadow.blur_radius)
                          ? std::max(0.0F, draw.shadow.blur_radius) * 0.5F
                          : 0.0F;
  const float margin = sigma * 3.0F + 1.0F;
  const Rect quad{
      .origin = {shape.origin.x - margin, shape.origin.y - margin},
      .size = {shape.size.width + margin * 2.0F, shape.size.height + margin * 2.0F}};
  // Positions get the paint transform and the color the composed opacity;
  // the shadow is evaluated at the untransformed corner.
  const auto transformed = metal_rect_vertices(
      quad, draw.shadow.color, draw.metadata, draw.composition_stack);
  if (transformed.front().color.w <= 0.0F) return std::nullopt;
  const float right = quad.origin.x + quad.size.width;
  const float bottom = quad.origin.y + quad.size.height;
  const std::array<simd_float2, 4> local{{
      {quad.origin.x, quad.origin.y}, {right, quad.origin.y},
      {quad.origin.x, bottom}, {right, bottom}}};
  MetalBoxShadowQuad result{
      .uniforms = {.shadow_rect = bounds(shape),
                   .shadow_radii = corners(shape_radii),
                   .box_rect = bounds(draw.rect),
                   .box_radii = corners(box_radii),
                   .sigma = sigma}};
  for (std::size_t index = 0; index < 4; ++index) {
    result.vertices[index] = {.position = transformed[index].position,
                              .local = local[index],
                              .color = transformed[index].color};
  }
  return result;
}

Result<void> metal_encode_box_shadow(
    id<MTLRenderCommandEncoder> encoder,
    MetalRendererState& state,
    const BoxShadowDraw& shadow) {
  const std::optional<MetalBoxShadowQuad> quad = metal_box_shadow_quad(shadow);
  if (!quad) return {};
  const MetalClipResolution clip =
      metal_resolve_clip(state.framebuffer_size, shadow.clip_rect, shadow.clip_stack);
  if (!clip.visible) return {};
  const simd_float2 viewport{state.framebuffer_size.width,
                             state.framebuffer_size.height};
  [encoder setScissorRect:clip.scissor];
  [encoder setRenderPipelineState:state.pipelines.box_shadow];
  [encoder setVertexBytes:quad->vertices.data()
                   length:sizeof(quad->vertices)
                  atIndex:0];
  [encoder setVertexBytes:&viewport length:sizeof(viewport) atIndex:1];
  [encoder setFragmentBytes:&quad->uniforms length:sizeof(quad->uniforms) atIndex:0];
  [encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
  return {};
}

}  // namespace cgpui
