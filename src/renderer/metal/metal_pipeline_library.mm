#include "metal_renderer_internal.hpp"
#include "metal_shaders_source.hpp"

#include <string_view>

namespace cgpui {
namespace {

// The shaders are compiled from Slang one entry point per source
// (scripts/dev/compile_shaders.py), so each function has its own library.
Result<id<MTLFunction>> make_function(MetalRendererState& state,
                                      std::string_view name) {
  for (const CgpuiMetalShader& shader : kCgpuiMetalShaders) {
    if (name != shader.function) {
      continue;
    }
    NSError* error = nil;
    id<MTLLibrary> library = [state.device
        newLibraryWithSource:[NSString stringWithUTF8String:shader.source]
                     options:nil
                       error:&error];
    id<MTLFunction> function =
        library == nil ? nil
                       : [library newFunctionWithName:
                              [NSString stringWithUTF8String:shader.function]];
    if (function == nil) {
      return std::unexpected(Error{
          .code = ErrorCode::renderer_initialization_failed,
          .message = error == nil
                         ? "Metal shader compilation failed: " + std::string(name)
                         : std::string(error.localizedDescription.UTF8String)});
    }
    return function;
  }
  return std::unexpected(Error{
      .code = ErrorCode::renderer_initialization_failed,
      .message = "Metal shader not embedded: " + std::string(name)});
}

Result<id<MTLRenderPipelineState>> make_pipeline(
    MetalRendererState& state,
    std::string_view fragment_name) {
  auto fragment = make_function(state, fragment_name);
  if (!fragment) {
    return std::unexpected(fragment.error());
  }
  MTLRenderPipelineDescriptor* descriptor =
      [[MTLRenderPipelineDescriptor alloc] init];
  descriptor.vertexFunction = state.pipelines.vertex_function;
  descriptor.fragmentFunction = *fragment;
  descriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
  descriptor.colorAttachments[0].blendingEnabled = YES;
  descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
  descriptor.colorAttachments[0].destinationRGBBlendFactor =
      MTLBlendFactorOneMinusSourceAlpha;
  descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
  descriptor.colorAttachments[0].destinationAlphaBlendFactor =
      MTLBlendFactorOneMinusSourceAlpha;
  NSError* error = nil;
  id<MTLRenderPipelineState> pipeline =
      [state.device newRenderPipelineStateWithDescriptor:descriptor error:&error];
  if (pipeline == nil) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = error == nil
                       ? "Metal pipeline creation failed"
                       : std::string(error.localizedDescription.UTF8String)});
  }
  return pipeline;
}

id<MTLSamplerState> make_sampler(
    id<MTLDevice> device,
    MTLSamplerMinMagFilter filter) {
  MTLSamplerDescriptor* descriptor = [[MTLSamplerDescriptor alloc] init];
  descriptor.minFilter = filter;
  descriptor.magFilter = filter;
  descriptor.sAddressMode = MTLSamplerAddressModeClampToEdge;
  descriptor.tAddressMode = MTLSamplerAddressModeClampToEdge;
  return [device newSamplerStateWithDescriptor:descriptor];
}

}  // namespace

Result<void> initialize_metal_pipeline_library(MetalRendererState& state) {
  auto vertex = make_function(state, "cgpui_vertex");
  if (!vertex) return std::unexpected(vertex.error());
  state.pipelines.vertex_function = *vertex;
  auto solid = make_pipeline(state, "cgpui_solid_fragment");
  auto rounded = make_pipeline(state, "cgpui_rounded_fragment");
  auto glyph = make_pipeline(state, "cgpui_glyph_fragment");
  auto image = make_pipeline(state, "cgpui_image_fragment");
  if (!solid) return std::unexpected(solid.error());
  if (!rounded) return std::unexpected(rounded.error());
  if (!glyph) return std::unexpected(glyph.error());
  if (!image) return std::unexpected(image.error());
  state.pipelines.solid = *solid;
  state.pipelines.rounded = *rounded;
  state.pipelines.glyph = *glyph;
  state.pipelines.image = *image;
  state.pipelines.nearest_sampler =
      make_sampler(state.device, MTLSamplerMinMagFilterNearest);
  state.pipelines.linear_sampler =
      make_sampler(state.device, MTLSamplerMinMagFilterLinear);
  if (state.pipelines.nearest_sampler == nil ||
      state.pipelines.linear_sampler == nil) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Metal sampler creation failed"});
  }
  return {};
}

}  // namespace cgpui
