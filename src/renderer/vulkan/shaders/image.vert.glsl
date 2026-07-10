#version 450

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_image_uv;
layout(location = 2) in vec4 in_color;

layout(location = 0) out vec2 out_image_uv;
layout(location = 1) out vec4 out_color;

layout(push_constant) uniform ImagePushConstants {
  vec2 framebuffer_size;
} image_push_constants;

void main() {
  vec2 framebuffer_size = max(
      image_push_constants.framebuffer_size,
      vec2(1.0));
  vec2 normalized = in_position / framebuffer_size;
  gl_Position = vec4(
      normalized.x * 2.0 - 1.0,
      normalized.y * 2.0 - 1.0,
      0.0,
      1.0);
  out_image_uv = in_image_uv;
  out_color = in_color;
}
