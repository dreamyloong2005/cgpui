#include "metal_renderer_internal.hpp"
#include "metal_shaders_source.hpp"

namespace cgpui {
namespace {

NSString* metal_shader_source() {
  return [NSString stringWithUTF8String:kCgpuiMetalShaderSource];
}

Result<id<MTLRenderPipelineState>> make_pipeline(
    MetalRendererState& state,
    NSString* fragment_name) {
  MTLRenderPipelineDescriptor* descriptor =
      [[MTLRenderPipelineDescriptor alloc] init];
  descriptor.vertexFunction =
      [state.pipelines.library newFunctionWithName:@"cgpui_vertex"];
  descriptor.fragmentFunction =
      [state.pipelines.library newFunctionWithName:fragment_name];
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
  NSError* error = nil;
  state.pipelines.library =
      [state.device newLibraryWithSource:metal_shader_source()
                                 options:nil
                                   error:&error];
  if (state.pipelines.library == nil) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = error == nil
                       ? "Metal shader compilation failed"
                       : std::string(error.localizedDescription.UTF8String)});
  }
  auto solid = make_pipeline(state, @"cgpui_solid_fragment");
  auto rounded = make_pipeline(state, @"cgpui_rounded_fragment");
  auto glyph = make_pipeline(state, @"cgpui_glyph_fragment");
  auto image = make_pipeline(state, @"cgpui_image_fragment");
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
