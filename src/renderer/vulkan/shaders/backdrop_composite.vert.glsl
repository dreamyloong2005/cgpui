#version 450

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_local;
layout(location = 2) in vec4 in_rect;
layout(location = 3) in vec4 in_radii;
layout(location = 4) in vec4 in_tint;
layout(location = 5) in vec4 in_region;

layout(location = 0) out vec2 out_local;
layout(location = 1) flat out vec4 out_rect;
layout(location = 2) flat out vec4 out_radii;
layout(location = 3) flat out vec4 out_tint;
layout(location = 4) flat out vec4 out_region;

layout(push_constant) uniform BackdropPushConstants {
  vec2 framebuffer_size;
  ivec2 source_origin;
  ivec2 source_max;
  ivec2 direction;
  int factor;
  int radius;
  float sigma;
} push_constants;

void main() {
  vec2 normalized = in_position / max(push_constants.framebuffer_size, vec2(1.0));
  gl_Position = vec4(normalized * 2.0 - 1.0, 0.0, 1.0);
  out_local = in_local;
  out_rect = in_rect;
  out_radii = in_radii;
  out_tint = in_tint;
  out_region = in_region;
}
