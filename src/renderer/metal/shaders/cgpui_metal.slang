// Metal renderer shaders (metal_*.mm). Compiled one entry point at a time by
// scripts/dev/compile_shaders.py into src/renderer/metal/generated/; the
// registers give the indices metal_*.mm binds: vertex buffer(0) vertices,
// buffer(1) viewport; fragment buffer(0) uniforms, texture(0), sampler(0).
// Box shadow: vertex buffer(0) shadow vertices, buffer(1) viewport;
// fragment buffer(0) shadow uniforms (metal_box_shadow.mm).
import cgpui_shared;
import cgpui_box_shadow;

struct VertexIn {
  float2 position;
  float2 texture_coordinate;
  float4 color;
};

struct VertexOut {
  float4 position : SV_Position;
  float2 texture_coordinate : TEXCOORD0;
  float4 color : COLOR0;
};

struct RoundedRectUniforms {
  float4 fill_color;
  float4 border_color;
  float4 radii;
  float2 size;
  float border_width;
  float fill_enabled;
};

StructuredBuffer<VertexIn> vertices : register(t0);
ConstantBuffer<float2> viewport : register(b1);
ConstantBuffer<RoundedRectUniforms> uniforms : register(b0);
Texture2D<float4> source_texture : register(t0);
SamplerState source_sampler : register(s0);

[shader("vertex")]
VertexOut cgpui_vertex(uint vertex_id : SV_VertexID) {
  VertexIn input = vertices[vertex_id];
  VertexOut output;
  output.position = float4(input.position.x / viewport.x * 2.0 - 1.0,
                           1.0 - input.position.y / viewport.y * 2.0, 0.0, 1.0);
  output.texture_coordinate = input.texture_coordinate;
  output.color = input.color;
  return output;
}

[shader("fragment")]
float4 cgpui_solid_fragment(VertexOut input) : SV_Target0 {
  return input.color;
}

bool rounded_contains(float2 point, float2 size, float4 radii) {
  bool left = point.x < size.x * 0.5;
  bool top = point.y < size.y * 0.5;
  float radius = top ? (left ? radii.x : radii.y)
                     : (left ? radii.w : radii.z);
  radius = clamp(radius, 0.0, min(size.x, size.y) * 0.5);
  float2 center = float2(left ? radius : size.x - radius,
                         top ? radius : size.y - radius);
  bool in_corner = (left ? point.x < radius : point.x > size.x - radius) &&
                   (top ? point.y < radius : point.y > size.y - radius);
  return !in_corner || distance(point, center) <= radius;
}

[shader("fragment")]
float4 cgpui_rounded_fragment(VertexOut input) : SV_Target0 {
  float2 point = input.texture_coordinate * uniforms.size;
  if (!rounded_contains(point, uniforms.size, uniforms.radii)) {
    discard;
  }
  float border = max(0.0, uniforms.border_width);
  if (border > 0.0) {
    float2 inner_size = max(float2(0.0), uniforms.size - border * 2.0);
    float2 inner_point = point - border;
    float4 inner_radii = max(float4(0.0), uniforms.radii - border);
    bool in_inner_bounds = all(inner_point >= 0.0) && all(inner_point <= inner_size);
    if (!in_inner_bounds || !rounded_contains(inner_point, inner_size, inner_radii)) {
      return uniforms.border_color;
    }
  }
  if (uniforms.fill_enabled < 0.5) {
    discard;
  }
  return uniforms.fill_color;
}

[shader("fragment")]
float4 cgpui_glyph_fragment(VertexOut input) : SV_Target0 {
  return glyph_color(
      input.color,
      source_texture.Sample(source_sampler, input.texture_coordinate).r);
}

[shader("fragment")]
float4 cgpui_image_fragment(VertexOut input) : SV_Target0 {
  return source_texture.Sample(source_sampler, input.texture_coordinate) *
         input.color;
}

// One shadow is one quad: `position` is the transformed device position,
// `local` the untransformed point the shadow is evaluated at.
struct BoxShadowVertexIn {
  float2 position;
  float2 local;
  float4 color;
};

struct BoxShadowVertexOut {
  float4 position : SV_Position;
  float2 local : TEXCOORD0;
  float4 color : COLOR0;
};

struct BoxShadowUniforms {
  float4 shadow_rect;
  float4 shadow_radii;
  float4 box_rect;
  float4 box_radii;
  float sigma;
};

StructuredBuffer<BoxShadowVertexIn> shadow_vertices : register(t0);
ConstantBuffer<BoxShadowUniforms> shadow : register(b0);

[shader("vertex")]
BoxShadowVertexOut cgpui_box_shadow_vertex(uint vertex_id : SV_VertexID) {
  BoxShadowVertexIn input = shadow_vertices[vertex_id];
  BoxShadowVertexOut output;
  output.position = float4(input.position.x / viewport.x * 2.0 - 1.0,
                           1.0 - input.position.y / viewport.y * 2.0, 0.0, 1.0);
  output.local = input.local;
  output.color = input.color;
  return output;
}

[shader("fragment")]
float4 cgpui_box_shadow_fragment(BoxShadowVertexOut input) : SV_Target0 {
  float alpha = box_shadow_alpha(input.local, shadow.shadow_rect,
                                 shadow.shadow_radii, shadow.box_rect,
                                 shadow.box_radii, shadow.sigma);
  return float4(input.color.rgb, input.color.a * alpha);
}

// Backdrop blur composite (metal_backdrop_blur.mm): the blurred scene
// region under a rounded mask, mixed with the tint. Vertex function
// cgpui_box_shadow_vertex; `local` is the untransformed point, the color's
// alpha the composed opacity. Fragment buffer(0) uniforms, texture(0) the
// blurred scene (sampled at the fragment's device position), sampler(0).
struct BackdropUniforms {
  float4 rect;
  float4 radii;
  float4 tint;
};

ConstantBuffer<BackdropUniforms> backdrop : register(b0);

[shader("fragment")]
float4 cgpui_backdrop_fragment(BoxShadowVertexOut input) : SV_Target0 {
  float mask = clamp(
      0.5 - rounded_rect_distance(input.local, backdrop.rect, backdrop.radii) /
                derivative_pixel_width(input.local),
      0.0, 1.0);
  float width;
  float height;
  source_texture.GetDimensions(width, height);
  float4 blurred = source_texture.Sample(
      source_sampler, input.position.xy / float2(width, height));
  float3 color = lerp(blurred.rgb, backdrop.tint.rgb, backdrop.tint.a);
  return float4(color, mask * input.color.a);
}
