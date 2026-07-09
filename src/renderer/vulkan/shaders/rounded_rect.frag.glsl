#version 450

layout(location = 0) in vec4 in_color;
layout(location = 1) in float in_coverage;
layout(location = 0) out vec4 out_color;

void main() {
  out_color = vec4(
      in_color.rgb,
      in_color.a * clamp(in_coverage, 0.0, 1.0));
}
