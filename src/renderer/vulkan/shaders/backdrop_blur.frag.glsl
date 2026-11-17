#version 450

// One separable Gaussian pass (sigma in texels) along direction, clamped
// to the region texels [0, source_max].
layout(set = 0, binding = 0) uniform sampler2D source_image;
layout(location = 0) out vec4 out_color;

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
  ivec2 texel = ivec2(gl_FragCoord.xy);
  float sigma = max(push_constants.sigma, 0.0001);
  vec4 sum = vec4(0.0);
  float total = 0.0;
  for (int i = -push_constants.radius; i <= push_constants.radius; ++i) {
    float weight = exp(-float(i * i) / (2.0 * sigma * sigma));
    ivec2 p = clamp(texel + push_constants.direction * i, ivec2(0),
                    push_constants.source_max);
    sum += texelFetch(source_image, p, 0) * weight;
    total += weight;
  }
  out_color = sum / total;
}
