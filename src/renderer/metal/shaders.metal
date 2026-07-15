#include <metal_stdlib>
using namespace metal;

struct VertexIn {
  float2 position;
  float2 texture_coordinate;
  float4 color;
};

struct VertexOut {
  float4 position [[position]];
  float2 texture_coordinate;
  float4 color;
};

struct RoundedRectUniforms {
  float4 fill_color;
  float4 border_color;
  float4 radii;
  float2 size;
  float border_width;
  float fill_enabled;
};

vertex VertexOut cgpui_vertex(
    uint vertex_id [[vertex_id]],
    constant VertexIn* vertices [[buffer(0)]],
    constant float2& viewport [[buffer(1)]]) {
  VertexIn input = vertices[vertex_id];
  VertexOut output;
  output.position = float4(
      input.position.x / viewport.x * 2.0 - 1.0,
      1.0 - input.position.y / viewport.y * 2.0,
      0.0,
      1.0);
  output.texture_coordinate = input.texture_coordinate;
  output.color = input.color;
  return output;
}

fragment float4 cgpui_solid_fragment(VertexOut input [[stage_in]]) {
  return input.color;
}

bool rounded_contains(float2 point, float2 size, float4 radii) {
  bool left = point.x < size.x * 0.5;
  bool top = point.y < size.y * 0.5;
  float radius = top ? (left ? radii.x : radii.y)
                     : (left ? radii.w : radii.z);
  radius = clamp(radius, 0.0, min(size.x, size.y) * 0.5);
  float2 center = float2(
      left ? radius : size.x - radius,
      top ? radius : size.y - radius);
  bool in_corner = (left ? point.x < radius : point.x > size.x - radius) &&
                   (top ? point.y < radius : point.y > size.y - radius);
  return !in_corner || distance(point, center) <= radius;
}

fragment float4 cgpui_rounded_fragment(
    VertexOut input [[stage_in]],
    constant RoundedRectUniforms& uniforms [[buffer(0)]]) {
  float2 point = input.texture_coordinate * uniforms.size;
  if (!rounded_contains(point, uniforms.size, uniforms.radii)) {
    discard_fragment();
  }
  float border = max(0.0, uniforms.border_width);
  if (border > 0.0) {
    float2 inner_size = max(float2(0.0), uniforms.size - border * 2.0);
    float2 inner_point = point - border;
    float4 inner_radii = max(float4(0.0), uniforms.radii - border);
    bool in_inner_bounds = all(inner_point >= 0.0) &&
                           all(inner_point <= inner_size);
    if (!in_inner_bounds ||
        !rounded_contains(inner_point, inner_size, inner_radii)) {
      return uniforms.border_color;
    }
  }
  if (uniforms.fill_enabled < 0.5) discard_fragment();
  return uniforms.fill_color;
}

fragment float4 cgpui_glyph_fragment(
    VertexOut input [[stage_in]],
    texture2d<float> glyph [[texture(0)]],
    sampler glyph_sampler [[sampler(0)]]) {
  float coverage = glyph.sample(glyph_sampler, input.texture_coordinate).r;
  return float4(input.color.rgb, input.color.a * coverage);
}

fragment float4 cgpui_image_fragment(
    VertexOut input [[stage_in]],
    texture2d<float> image [[texture(0)]],
    sampler image_sampler [[sampler(0)]]) {
  return image.sample(image_sampler, input.texture_coordinate) * input.color;
}
