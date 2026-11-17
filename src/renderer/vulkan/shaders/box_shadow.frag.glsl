#version 450

// Outer box shadow: a rounded rect convolved with a Gaussian (sigma), using
// the closed-form erf integration from Evan Wallace's "Fast rounded
// rectangle shadows" (also used by GPUI/Zed), clipped out by the box.

layout(location = 0) in vec2 in_local;
layout(location = 1) flat in vec4 in_color;
layout(location = 2) flat in vec4 in_shadow_rect;
layout(location = 3) flat in vec4 in_shadow_radii;
layout(location = 4) flat in vec4 in_box_rect;
layout(location = 5) flat in vec4 in_box_radii;
layout(location = 6) flat in float in_sigma;
layout(location = 0) out vec4 out_color;

// Radii are {top left, top right, bottom right, bottom left}; y points down.
float corner_radius(vec2 p, vec4 radii) {
  if (p.x < 0.0) {
    return p.y < 0.0 ? radii.x : radii.w;
  }
  return p.y < 0.0 ? radii.y : radii.z;
}

float rounded_rect_distance(vec2 point, vec4 rect, vec4 radii) {
  vec2 half_size = (rect.zw - rect.xy) * 0.5;
  vec2 p = point - (rect.xy + rect.zw) * 0.5;
  float r = corner_radius(p, radii);
  vec2 q = abs(p) - half_size + r;
  return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

// One device pixel measured in local units, so pixel-aligned edges are
// exact and the antialiasing width follows the transform scale.
float edge_coverage(float distance) {
  float width = max(length(fwidth(in_local)) * 0.7071068, 0.0001);
  return clamp(0.5 - distance / width, 0.0, 1.0);
}

vec2 erf_approx(vec2 x) {
  vec2 s = sign(x);
  vec2 a = abs(x);
  vec2 r = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
  r *= r;
  return s - s / (r * r);
}

float gaussian(float x, float sigma) {
  return exp(-(x * x) / (2.0 * sigma * sigma)) / (2.5066283 * sigma);
}

float shadow_x(float x, float y, float sigma, float corner, vec2 half_size) {
  float delta = min(half_size.y - corner - abs(y), 0.0);
  float curved =
      half_size.x - corner + sqrt(max(0.0, corner * corner - delta * delta));
  vec2 integral =
      0.5 + 0.5 * erf_approx((x + vec2(-curved, curved)) * (0.7071068 / sigma));
  return integral.y - integral.x;
}

float blurred_coverage(vec2 point, vec4 rect, vec4 radii, float sigma) {
  vec2 half_size = (rect.zw - rect.xy) * 0.5;
  vec2 p = point - (rect.xy + rect.zw) * 0.5;
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

void main() {
  float shadow = in_sigma < 0.05
      ? edge_coverage(rounded_rect_distance(in_local, in_shadow_rect,
                                            in_shadow_radii))
      : blurred_coverage(in_local, in_shadow_rect, in_shadow_radii, in_sigma);
  float box = edge_coverage(
      rounded_rect_distance(in_local, in_box_rect, in_box_radii));
  out_color = vec4(
      in_color.rgb,
      in_color.a * clamp(shadow, 0.0, 1.0) * (1.0 - box));
}
