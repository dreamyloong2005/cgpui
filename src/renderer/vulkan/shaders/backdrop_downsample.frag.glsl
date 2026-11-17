#version 450

// Box-filters factor x factor scene pixels of the backdrop region into one
// texel; the region's top-left scene pixel is source_origin.
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
  ivec2 base = push_constants.source_origin + texel * push_constants.factor;
  vec4 sum = vec4(0.0);
  for (int y = 0; y < push_constants.factor; ++y) {
    for (int x = 0; x < push_constants.factor; ++x) {
      ivec2 p = clamp(base + ivec2(x, y), push_constants.source_origin,
                      push_constants.source_max);
      sum += texelFetch(source_image, p, 0);
    }
  }
  out_color = sum / float(push_constants.factor * push_constants.factor);
}
