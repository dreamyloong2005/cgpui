// Text pipeline (vulkan_text_*): glyph atlas quads.
import cgpui_shared;

struct PushConstants {
  float2 framebuffer_size;
};
[[vk::push_constant]] ConstantBuffer<PushConstants> push_constants;

[[vk::binding(0, 0)]] Sampler2D glyph_atlas;

struct VertexInput {
  [[vk::location(0)]] float2 position : POSITION;
  [[vk::location(1)]] float2 atlas_uv : TEXCOORD;
  [[vk::location(2)]] float4 color : COLOR;
};

struct Varyings {
  float4 position : SV_Position;
  [[vk::location(0)]] float2 atlas_uv : TEXCOORD;
  [[vk::location(1)]] float4 color : COLOR;
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
  output.position.y = -output.position.y;
#endif
  output.atlas_uv = input.atlas_uv;
  output.color = input.color;
  return output;
}

[shader("fragment")]
float4 fragment_main(Varyings input) : SV_Target0 {
  return glyph_color(input.color, glyph_atlas.Sample(input.atlas_uv).r);
}
