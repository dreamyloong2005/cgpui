#include "metal_effects_internal.hpp"

#import <MetalPerformanceShaders/MetalPerformanceShaders.h>

#include <algorithm>
#include <cmath>

namespace cgpui {
namespace {

id<MTLTexture> make_target(id<MTLDevice> device, NSUInteger width, NSUInteger height) {
  MTLTextureDescriptor* descriptor = [MTLTextureDescriptor
      texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                   width:width
                                  height:height
                               mipmapped:NO];
  descriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead |
                     MTLTextureUsageShaderWrite;
  descriptor.storageMode = MTLStorageModePrivate;
  return [device newTextureWithDescriptor:descriptor];
}

id<MTLRenderCommandEncoder> load_encoder(id<MTLCommandBuffer> command_buffer,
                                         id<MTLTexture> target) {
  MTLRenderPassDescriptor* pass = [MTLRenderPassDescriptor renderPassDescriptor];
  pass.colorAttachments[0].texture = target;
  pass.colorAttachments[0].loadAction = MTLLoadActionLoad;
  pass.colorAttachments[0].storeAction = MTLStoreActionStore;
  return [command_buffer renderCommandEncoderWithDescriptor:pass];
}

Error encoder_error() {
  return Error{.code = ErrorCode::frame_acquisition_failed,
               .message = "Metal backdrop blur encoder creation failed"};
}

}  // namespace

Result<void> metal_ensure_backdrop_targets(
    MetalRendererState& state,
    NSUInteger width,
    NSUInteger height) {
  if (state.backdrop_scene != nil && state.backdrop_scene.width == width &&
      state.backdrop_scene.height == height) {
    return {};
  }
  state.backdrop_scene = make_target(state.device, width, height);
  state.backdrop_blurred = make_target(state.device, width, height);
  if (state.backdrop_scene == nil || state.backdrop_blurred == nil) {
    state.backdrop_scene = nil;
    state.backdrop_blurred = nil;
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Metal backdrop blur target creation failed"});
  }
  return {};
}

Result<id<MTLRenderCommandEncoder>> metal_encode_backdrop_blur(
    id<MTLCommandBuffer> command_buffer,
    id<MTLRenderCommandEncoder> encoder,
    MetalRendererState& state,
    const BackdropBlurDraw& backdrop) {
  [encoder endEncoding];
  id<MTLTexture> scene = state.backdrop_scene;
  const auto vertices = metal_rect_vertices(
      backdrop.rect, Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
      backdrop.metadata, backdrop.composition_stack);
  // The blurred region: the transformed rect's bounds plus 3 sigma, inside
  // the drawable. Sigma is the blur radius in device pixels.
  const float sigma = std::isfinite(backdrop.blur_radius)
                          ? std::max(0.0F, backdrop.blur_radius)
                          : 0.0F;
  float left = vertices[0].position.x;
  float top = vertices[0].position.y;
  float right = left;
  float bottom = top;
  for (const MetalVertex& vertex : vertices) {
    left = std::min(left, vertex.position.x);
    right = std::max(right, vertex.position.x);
    top = std::min(top, vertex.position.y);
    bottom = std::max(bottom, vertex.position.y);
  }
  const auto clamp_x = [&](float value) {
    return static_cast<NSUInteger>(std::clamp(value, 0.0F, static_cast<float>(scene.width)));
  };
  const auto clamp_y = [&](float value) {
    return static_cast<NSUInteger>(std::clamp(value, 0.0F, static_cast<float>(scene.height)));
  };
  const NSUInteger x0 = clamp_x(std::floor(left - sigma * 3.0F));
  const NSUInteger y0 = clamp_y(std::floor(top - sigma * 3.0F));
  const NSUInteger x1 = clamp_x(std::ceil(right + sigma * 3.0F));
  const NSUInteger y1 = clamp_y(std::ceil(bottom + sigma * 3.0F));
  if (x1 > x0 && y1 > y0) {
    if (sigma >= 0.05F) {
      MPSImageGaussianBlur* blur =
          [[MPSImageGaussianBlur alloc] initWithDevice:state.device sigma:sigma];
      blur.edgeMode = MPSImageEdgeModeClamp;
      blur.clipRect = MTLRegionMake2D(x0, y0, x1 - x0, y1 - y0);
      [blur encodeToCommandBuffer:command_buffer
                    sourceTexture:scene
               destinationTexture:state.backdrop_blurred];
    } else {
      id<MTLBlitCommandEncoder> blit = [command_buffer blitCommandEncoder];
      [blit copyFromTexture:scene sourceSlice:0 sourceLevel:0
               sourceOrigin:MTLOriginMake(x0, y0, 0)
                 sourceSize:MTLSizeMake(x1 - x0, y1 - y0, 1)
                  toTexture:state.backdrop_blurred destinationSlice:0
           destinationLevel:0 destinationOrigin:MTLOriginMake(x0, y0, 0)];
      [blit endEncoding];
    }
  }
  id<MTLRenderCommandEncoder> next = load_encoder(command_buffer, scene);
  if (next == nil) return std::unexpected(encoder_error());
  const MetalClipResolution clip =
      metal_resolve_clip(state.framebuffer_size, backdrop.clip_rect, backdrop.clip_stack);
  if (!clip.visible || x1 <= x0 || y1 <= y0) return next;
  const float rect_right = backdrop.rect.origin.x + backdrop.rect.size.width;
  const float rect_bottom = backdrop.rect.origin.y + backdrop.rect.size.height;
  const std::array<simd_float2, 4> local{{
      {backdrop.rect.origin.x, backdrop.rect.origin.y}, {rect_right, backdrop.rect.origin.y},
      {backdrop.rect.origin.x, rect_bottom}, {rect_right, rect_bottom}}};
  std::array<MetalBoxShadowVertex, 4> quad{};
  for (std::size_t index = 0; index < 4; ++index) {
    quad[index] = {.position = vertices[index].position, .local = local[index],
                   .color = vertices[index].color};
  }
  const float limit = std::min(backdrop.rect.size.width, backdrop.rect.size.height) * 0.5F;
  const auto radius = [&](float value) { return std::clamp(value, 0.0F, limit); };
  const Color tint = backdrop.tint.value_or(Color{.a = 0.0F});
  const MetalBackdropUniforms uniforms{
      .rect = {backdrop.rect.origin.x, backdrop.rect.origin.y, rect_right, rect_bottom},
      .radii = {radius(backdrop.radius.top_left), radius(backdrop.radius.top_right),
                radius(backdrop.radius.bottom_right), radius(backdrop.radius.bottom_left)},
      .tint = {tint.r, tint.g, tint.b, tint.a}};
  const simd_float2 viewport{state.framebuffer_size.width, state.framebuffer_size.height};
  [next setScissorRect:clip.scissor];
  [next setRenderPipelineState:state.pipelines.backdrop];
  [next setVertexBytes:quad.data() length:sizeof(quad) atIndex:0];
  [next setVertexBytes:&viewport length:sizeof(viewport) atIndex:1];
  [next setFragmentBytes:&uniforms length:sizeof(uniforms) atIndex:0];
  [next setFragmentTexture:state.backdrop_blurred atIndex:0];
  [next setFragmentSamplerState:state.pipelines.nearest_sampler atIndex:0];
  [next drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
  return next;
}

Result<void> metal_encode_scene_copy(
    id<MTLCommandBuffer> command_buffer,
    MetalRendererState& state,
    id<MTLTexture> drawable) {
  MTLRenderPassDescriptor* pass = [MTLRenderPassDescriptor renderPassDescriptor];
  pass.colorAttachments[0].texture = drawable;
  pass.colorAttachments[0].loadAction = MTLLoadActionDontCare;
  pass.colorAttachments[0].storeAction = MTLStoreActionStore;
  id<MTLRenderCommandEncoder> encoder =
      [command_buffer renderCommandEncoderWithDescriptor:pass];
  if (encoder == nil) return std::unexpected(encoder_error());
  const auto vertices = metal_rect_vertices(
      Rect{.size = state.framebuffer_size}, Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
      {}, {});
  const simd_float2 viewport{state.framebuffer_size.width, state.framebuffer_size.height};
  [encoder setRenderPipelineState:state.pipelines.copy];
  [encoder setVertexBytes:vertices.data() length:sizeof(vertices) atIndex:0];
  [encoder setVertexBytes:&viewport length:sizeof(viewport) atIndex:1];
  [encoder setFragmentTexture:state.backdrop_scene atIndex:0];
  [encoder setFragmentSamplerState:state.pipelines.nearest_sampler atIndex:0];
  [encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
  [encoder endEncoding];
  return {};
}

}  // namespace cgpui
