#version 450

layout(set = 0, binding = 0) uniform sampler2D glyph_atlas;

layout(location = 0) in vec2 in_atlas_uv;
layout(location = 1) in vec4 in_color;

layout(location = 0) out vec4 out_color;

void main() {
  const float coverage_gamma = 1.0;
  float coverage = pow(
      clamp(texture(glyph_atlas, in_atlas_uv).r, 0.0, 1.0),
      coverage_gamma);
  out_color = vec4(in_color.rgb, in_color.a * coverage);
}
