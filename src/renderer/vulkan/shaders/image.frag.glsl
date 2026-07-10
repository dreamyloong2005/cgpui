#version 450

layout(set = 0, binding = 0) uniform sampler2D image_texture;

layout(location = 0) in vec2 in_image_uv;
layout(location = 1) in vec4 in_color;

layout(location = 0) out vec4 out_color;

void main() {
  out_color = texture(image_texture, in_image_uv) * in_color;
}
