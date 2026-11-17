#version 450

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_local;
layout(location = 2) in vec4 in_color;
layout(location = 3) in vec4 in_shadow_rect;
layout(location = 4) in vec4 in_shadow_radii;
layout(location = 5) in vec4 in_box_rect;
layout(location = 6) in vec4 in_box_radii;
layout(location = 7) in float in_sigma;

layout(location = 0) out vec2 out_local;
layout(location = 1) flat out vec4 out_color;
layout(location = 2) flat out vec4 out_shadow_rect;
layout(location = 3) flat out vec4 out_shadow_radii;
layout(location = 4) flat out vec4 out_box_rect;
layout(location = 5) flat out vec4 out_box_radii;
layout(location = 6) flat out float out_sigma;

layout(push_constant) uniform BoxShadowPushConstants {
  vec2 framebuffer_size;
} box_shadow_push_constants;

void main() {
  vec2 framebuffer_size = max(
      box_shadow_push_constants.framebuffer_size,
      vec2(1.0));
  vec2 normalized = in_position / framebuffer_size;
  gl_Position = vec4(
      normalized.x * 2.0 - 1.0,
      normalized.y * 2.0 - 1.0,
      0.0,
      1.0);
  out_local = in_local;
  out_color = in_color;
  out_shadow_rect = in_shadow_rect;
  out_shadow_radii = in_shadow_radii;
  out_box_rect = in_box_rect;
  out_box_radii = in_box_radii;
  out_sigma = in_sigma;
}
