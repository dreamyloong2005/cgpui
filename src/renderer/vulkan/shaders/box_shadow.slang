// Box shadow pipeline (vulkan_box_shadow_*). Outer box shadow: a rounded
// rect convolved with a Gaussian (sigma), using the closed-form erf
// integration from Evan Wallace's "Fast rounded rectangle shadows" (also used
// by GPUI/Zed), clipped out by the box.
import cgpui_shared;

struct PushConstants {
  float2 framebuffer_size;
};
[[vk::push_constant]] ConstantBuffer<PushConstants> push_constants;

struct VertexInput {
  [[vk::location(0)]] float2 position : POSITION;
  [[vk::location(1)]] float2 local : LOCAL;
  [[vk::location(2)]] float4 color : COLOR;
  [[vk::location(3)]] float4 shadow_rect : SHADOW_RECT;
  [[vk::location(4)]] float4 shadow_radii : SHADOW_RADII;
  [[vk::location(5)]] float4 box_rect : BOX_RECT;
  [[vk::location(6)]] float4 box_radii : BOX_RADII;
  [[vk::location(7)]] float sigma : SIGMA;
};

struct Varyings {
  float4 position : SV_Position;
  [[vk::location(0)]] float2 local : LOCAL;
  [[vk::location(1)]] nointerpolation float4 color : COLOR;
  [[vk::location(2)]] nointerpolation float4 shadow_rect : SHADOW_RECT;
  [[vk::location(3)]] nointerpolation float4 shadow_radii : SHADOW_RADII;
  [[vk::location(4)]] nointerpolation float4 box_rect : BOX_RECT;
  [[vk::location(5)]] nointerpolation float4 box_radii : BOX_RADII;
  [[vk::location(6)]] nointerpolation float sigma : SIGMA;
};

[shader("vertex")]
Varyings vertex_main(VertexInput input) {
  float2 framebuffer_size = max(push_constants.framebuffer_size, float2(1.0));
  float2 normalized = input.position / framebuffer_size;
  Varyings output;
  // Vulkan clip space has y down, like the framebuffer: no flip.
  output.position = float4(normalized.x * 2.0 - 1.0,
                           normalized.y * 2.0 - 1.0, 0.0, 1.0);
#if CGPUI_CLIP_Y_UP
  output.position.y = -output.position.y;
#endif
  output.local = input.local;
  output.color = input.color;
  output.shadow_rect = input.shadow_rect;
  output.shadow_radii = input.shadow_radii;
  output.box_rect = input.box_rect;
  output.box_radii = input.box_radii;
  output.sigma = input.sigma;
  return output;
}

float edge_coverage(float distance, float2 local) {
  return clamp(0.5 - distance / local_pixel_width(local), 0.0, 1.0);
}

float2 erf_approx(float2 x) {
  float2 s = sign(x);
  float2 a = abs(x);
  float2 r = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
  r *= r;
  return s - s / (r * r);
}

float gaussian(float x, float sigma) {
  return exp(-(x * x) / (2.0 * sigma * sigma)) / (2.5066283 * sigma);
}

float shadow_x(float x, float y, float sigma, float corner, float2 half_size) {
  float delta = min(half_size.y - corner - abs(y), 0.0);
  float curved =
      half_size.x - corner + sqrt(max(0.0, corner * corner - delta * delta));
  float2 integral =
      0.5 + 0.5 * erf_approx((x + float2(-curved, curved)) * (0.7071068 / sigma));
  return integral.y - integral.x;
}

float blurred_coverage(float2 point, float4 rect, float4 radii, float sigma) {
  float2 half_size = (rect.zw - rect.xy) * 0.5;
  float2 p = point - (rect.xy + rect.zw) * 0.5;
  float corner = corner_radius(p, radii);
  float low = p.y - half_size.y;
  float high = p.y + half_size.y;
  float start = clamp(-3.0 * sigma, low, high);
  float end = clamp(3.0 * sigma, low, high);
  float step_size = (end - start) / 4.0;
  float y = start + step_size * 0.5;
  float value = 0.0;
  for (int i = 0; i < 4; ++i) {
    value += shadow_x(p.x, p.y - y, sigma, corner, half_size) *
             gaussian(y, sigma) * step_size;
    y += step_size;
  }
  return value;
}

[shader("fragment")]
float4 fragment_main(Varyings input) : SV_Target0 {
  float shadow = input.sigma < 0.05
      ? edge_coverage(rounded_rect_distance(input.local, input.shadow_rect,
                                            input.shadow_radii), input.local)
      : blurred_coverage(input.local, input.shadow_rect, input.shadow_radii,
                         input.sigma);
  float box = edge_coverage(
      rounded_rect_distance(input.local, input.box_rect, input.box_radii),
      input.local);
  return float4(input.color.rgb,
                input.color.a * clamp(shadow, 0.0, 1.0) * (1.0 - box));
}
