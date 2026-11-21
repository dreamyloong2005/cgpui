// Outer box shadow coverage shared by the Metal shader (cgpui_metal.slang).
// The same math as src/renderer/vulkan/shaders/box_shadow.slang: a rounded
// rect convolved with a Gaussian (sigma), with the closed-form erf
// integration from Evan Wallace's "Fast rounded rectangle shadows" (also
// used by GPUI/Zed), clipped out by the box. The Vulkan shader keeps its own
// copy until its SPIR-V is regenerated and validated with spirv-val.
module cgpui_box_shadow;

import cgpui_shared;

// local_pixel_width (cgpui_shared) spelled with ddx/ddy: fwidth is
// abs(ddx) + abs(ddy), and slangc 2026.1 does not accept fwidth for the
// Metal target.
public float derivative_pixel_width(float2 local) {
  return max(length(abs(ddx(local)) + abs(ddy(local))) * 0.7071068, 0.0001);
}

float edge_coverage(float distance, float2 local) {
  return clamp(0.5 - distance / derivative_pixel_width(local), 0.0, 1.0);
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

// Rects are {min x, min y, max x, max y}; radii {tl, tr, br, bl}.
public float box_shadow_alpha(float2 local, float4 shadow_rect,
                              float4 shadow_radii, float4 box_rect,
                              float4 box_radii, float sigma) {
  float shadow = sigma < 0.05
      ? edge_coverage(rounded_rect_distance(local, shadow_rect, shadow_radii), local)
      : blurred_coverage(local, shadow_rect, shadow_radii, sigma);
  float box = edge_coverage(rounded_rect_distance(local, box_rect, box_radii), local);
  return clamp(shadow, 0.0, 1.0) * (1.0 - box);
}
