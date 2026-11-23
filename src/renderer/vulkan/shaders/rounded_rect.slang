// Rounded rect pipeline (vulkan_rounded_rect_*): CPU-tessellated geometry
// with a per-vertex coverage for the antialiased edge.
struct PushConstants {
  float2 framebuffer_size;
};
[[vk::push_constant]] ConstantBuffer<PushConstants> push_constants;

struct VertexInput {
  [[vk::location(0)]] float2 position : POSITION;
  [[vk::location(1)]] float4 color : COLOR;
  [[vk::location(2)]] float coverage : COVERAGE;
};

struct Varyings {
  float4 position : SV_Position;
  [[vk::location(0)]] float4 color : COLOR;
  [[vk::location(1)]] float coverage : COVERAGE;
};

[shader("vertex")]
Varyings vertex_main(VertexInput input) {
  float2 framebuffer_size = max(push_constants.framebuffer_size, float2(1.0));
  float2 normalized = input.position / framebuffer_size;
  Varyings output;
  // Vulkan clip space has y down, like the framebuffer: no flip.
  output.position = float4(normalized.x * 2.0 - 1.0,
                           normalized.y * 2.0 - 1.0, 0.0, 1.0);
#if CGPUI_CLIP_Y_UP
  // D3D clip space points up; the shared CPU geometry remains y-down.
  output.position.y = -output.position.y;
#endif
  output.color = input.color;
  output.coverage = input.coverage;
  return output;
}

[shader("fragment")]
float4 fragment_main(Varyings input) : SV_Target0 {
  return float4(input.color.rgb,
                input.color.a * clamp(input.coverage, 0.0, 1.0));
}
