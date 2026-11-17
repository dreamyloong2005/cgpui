#version 450

// Copies the offscreen scene to the swapchain pixel for pixel.
layout(set = 0, binding = 0) uniform sampler2D scene_image;
layout(location = 0) out vec4 out_color;

void main() {
  out_color = texelFetch(scene_image, ivec2(gl_FragCoord.xy), 0);
}
