// Backdrop blur pipelines (vulkan_backdrop_blur_*): downsample, separable
// Gaussian passes, present and the rounded-rect composite. Every pass binds
// its source image at set 0, binding 0 and shares one push-constant block.
import cgpui_shared;

struct PushConstants {
  float2 framebuffer_size;
  int2 source_origin;
  // The last valid texel of the region a pass reads.
  int2 source_max;
  int2 direction;
  int factor;
  int radius;
  float sigma;
};
[[vk::push_constant]] ConstantBuffer<PushConstants> push_constants;

#if CGPUI_DX12
// Slang's combined Sampler2D.Load lowers to an invalid HLSL Texture.Load
// with a sampler argument. Keep DX12's native texture/sampler binding explicit.
Texture2D<float4> source_image : register(t0);
SamplerState source_sampler : register(s0);
// Only the DX12 alpha-preserving composite reads the unfiltered scene.
Texture2D<float4> scene_snapshot : register(t1);
#else
[[vk::binding(0, 0)]] Sampler2D source_image;
#endif

float4 fetch(int2 texel) {
  return source_image.Load(int3(texel, 0));
}

// One triangle covering the viewport; fragments use their SV_Position.
[shader("vertex")]
float4 fullscreen_vertex(uint vertex_index : SV_VertexID) : SV_Position {
  float2 corner = float2(float((vertex_index << 1) & 2), float(vertex_index & 2));
  return float4(corner * 2.0 - 1.0, 0.0, 1.0);
}

// Box-filters factor x factor scene pixels of the backdrop region into one
// texel; the region's top-left scene pixel is source_origin.
[shader("fragment")]
float4 downsample_fragment(float4 frag_coord : SV_Position) : SV_Target0 {
  int2 texel = int2(frag_coord.xy);
  int2 base = push_constants.source_origin + texel * push_constants.factor;
  float4 sum = float4(0.0);
  for (int y = 0; y < push_constants.factor; ++y) {
    for (int x = 0; x < push_constants.factor; ++x) {
      sum += fetch(clamp(base + int2(x, y), push_constants.source_origin,
                         push_constants.source_max));
    }
  }
  return sum / float(push_constants.factor * push_constants.factor);
}

// One separable Gaussian pass (sigma in texels) along direction, clamped to
// the region texels [0, source_max].
[shader("fragment")]
float4 blur_fragment(float4 frag_coord : SV_Position) : SV_Target0 {
  int2 texel = int2(frag_coord.xy);
  float sigma = max(push_constants.sigma, 0.0001);
  float4 sum = float4(0.0);
  float total = 0.0;
  for (int i = -push_constants.radius; i <= push_constants.radius; ++i) {
    float weight = exp(-float(i * i) / (2.0 * sigma * sigma));
    sum += fetch(clamp(texel + push_constants.direction * i, int2(0),
                       push_constants.source_max)) * weight;
    total += weight;
  }
  return sum / total;
}

// Copies the offscreen scene to the swapchain pixel for pixel.
[shader("fragment")]
float4 present_fragment(float4 frag_coord : SV_Position) : SV_Target0 {
  return fetch(int2(frag_coord.xy));
}

struct CompositeInput {
  [[vk::location(0)]] float2 position : POSITION;
  [[vk::location(1)]] float2 local : LOCAL;
  [[vk::location(2)]] float4 rect : RECT;
  [[vk::location(3)]] float4 radii : RADII;
  [[vk::location(4)]] float4 tint : TINT;
  [[vk::location(5)]] float4 region : REGION;
};

// region holds the blurred region's scene origin (xy), the downsample factor
// (z) and the composed opacity (w); the tint is mixed in by its alpha.
struct CompositeVaryings {
  float4 position : SV_Position;
  [[vk::location(0)]] float2 local : LOCAL;
  [[vk::location(1)]] nointerpolation float4 rect : RECT;
  [[vk::location(2)]] nointerpolation float4 radii : RADII;
  [[vk::location(3)]] nointerpolation float4 tint : TINT;
  [[vk::location(4)]] nointerpolation float4 region : REGION;
};

[shader("vertex")]
CompositeVaryings composite_vertex(CompositeInput input) {
  float2 normalized =
      input.position / max(push_constants.framebuffer_size, float2(1.0));
  CompositeVaryings output;
  output.position = float4(normalized * 2.0 - 1.0, 0.0, 1.0);
#if CGPUI_CLIP_Y_UP
  output.position.y = -output.position.y;
#endif
  output.local = input.local;
  output.rect = input.rect;
  output.radii = input.radii;
  output.tint = input.tint;
  output.region = input.region;
  return output;
}

// Draws the blurred backdrop inside the rounded rect.
[shader("fragment")]
float4 composite_fragment(CompositeVaryings input) : SV_Target0 {
  float mask = clamp(
      0.5 - rounded_rect_distance(input.local, input.rect, input.radii) /
                local_pixel_width(input.local),
      0.0, 1.0);
  float width;
  float height;
  source_image.GetDimensions(width, height);
  float2 texel = clamp((input.position.xy - input.region.xy) / input.region.z,
                       float2(0.5), float2(push_constants.source_max) + 0.5);
#if CGPUI_DX12
  float4 blurred = source_image.Sample(source_sampler, texel / float2(width, height));
#else
  float4 blurred = source_image.Sample(texel / float2(width, height));
#endif
  float3 color = lerp(blurred.rgb, input.tint.rgb, input.tint.a);
  return float4(color, mask * input.region.w);
}

#if CGPUI_DX12
// A premultiplied surface needs replacement, not another source-over layer:
// interpolate the original scene with its filtered/tinted value through the
// effect mask. Reusing the opaque composite would turn transparent black opaque.
[shader("fragment")]
float4 composite_alpha_fragment(CompositeVaryings input) : SV_Target0 {
  float mask = clamp(
      0.5 - rounded_rect_distance(input.local, input.rect, input.radii) /
                local_pixel_width(input.local),
      0.0, 1.0);
  float width;
  float height;
  source_image.GetDimensions(width, height);
  float2 texel = clamp((input.position.xy - input.region.xy) / input.region.z,
                       float2(0.5), float2(push_constants.source_max) + 0.5);
  float4 blurred = source_image.Sample(source_sampler, texel / float2(width, height));
  float4 scene = scene_snapshot.Load(int3(int2(input.position.xy), 0));
  // Tint is a straight-alpha color over the filtered premultiplied backdrop.
  float4 tinted = lerp(blurred, float4(input.tint.rgb, 1.0), input.tint.a);
  return lerp(scene, tinted, mask * input.region.w);
}
#endif
