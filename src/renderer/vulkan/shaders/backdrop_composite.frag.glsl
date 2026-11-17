#version 450

// Draws the blurred backdrop inside the rounded rect. in_region holds the
// blurred region's scene origin (xy), the downsample factor (z) and the
// composed opacity (w); the tint is mixed in by its alpha.
layout(set = 0, binding = 0) uniform sampler2D blurred_image;
layout(location = 0) in vec2 in_local;
layout(location = 1) flat in vec4 in_rect;
layout(location = 2) flat in vec4 in_radii;
layout(location = 3) flat in vec4 in_tint;
layout(location = 4) flat in vec4 in_region;
layout(location = 0) out vec4 out_color;

// source_max is the last valid texel of this backdrop's blurred region.
layout(push_constant) uniform BackdropPushConstants {
  vec2 framebuffer_size;
  ivec2 source_origin;
  ivec2 source_max;
  ivec2 direction;
  int factor;
  int radius;
  float sigma;
} push_constants;

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

void main() {
  float pixel = max(length(fwidth(in_local)) * 0.7071068, 0.0001);
  float mask = clamp(
      0.5 - rounded_rect_distance(in_local, in_rect, in_radii) / pixel,
      0.0, 1.0);
  vec2 size = vec2(textureSize(blurred_image, 0));
  vec2 texel = clamp((gl_FragCoord.xy - in_region.xy) / in_region.z,
                     vec2(0.5), vec2(push_constants.source_max) + 0.5);
  vec4 blurred = texture(blurred_image, texel / size);
  vec3 color = mix(blurred.rgb, in_tint.rgb, in_tint.a);
  out_color = vec4(color, mask * in_region.w);
}
