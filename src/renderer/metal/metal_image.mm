#include "metal_renderer_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

Result<MetalImageTextureResource> make_image_texture(
    MetalRendererState& state,
    const ImageAsset& image) {
  const auto& bitmap = image.bitmap;
  if (bitmap.width == 0 || bitmap.height == 0 ||
      bitmap.stride < bitmap.width * 4U ||
      bitmap.pixels.size() <
          static_cast<std::size_t>(bitmap.stride) * bitmap.height) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "Metal image upload requires complete RGBA8 pixels"});
  }
  MTLTextureDescriptor* descriptor = [MTLTextureDescriptor
      texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                   width:bitmap.width
                                  height:bitmap.height
                               mipmapped:NO];
  descriptor.usage = MTLTextureUsageShaderRead;
  descriptor.storageMode = MTLStorageModeShared;
  id<MTLTexture> texture = [state.device newTextureWithDescriptor:descriptor];
  if (texture == nil) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Metal image texture creation failed"});
  }
  [texture replaceRegion:MTLRegionMake2D(0, 0, bitmap.width, bitmap.height)
              mipmapLevel:0
                withBytes:bitmap.pixels.data()
              bytesPerRow:bitmap.stride];
  return MetalImageTextureResource{
      .texture = texture,
      .descriptor = describe_image_asset(image)};
}

Color image_tint(const ImageDraw& image) {
  return image.tint.value_or(
      Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F});
}

Rect image_uv(const ImageDraw& image) {
  if (!image.source_rect || image.asset.pixel_width == 0 ||
      image.asset.pixel_height == 0) {
    return {{}, {1.0F, 1.0F}};
  }
  const float width = static_cast<float>(image.asset.pixel_width);
  const float height = static_cast<float>(image.asset.pixel_height);
  const Rect source = *image.source_rect;
  const float left = std::clamp(source.origin.x / width, 0.0F, 1.0F);
  const float top = std::clamp(source.origin.y / height, 0.0F, 1.0F);
  const float right = std::clamp(
      (source.origin.x + source.size.width) / width, 0.0F, 1.0F);
  const float bottom = std::clamp(
      (source.origin.y + source.size.height) / height, 0.0F, 1.0F);
  return {{left, top}, {right - left, bottom - top}};
}

}  // namespace

Result<std::size_t> metal_upload_images(
    MetalRendererState& state,
    const MetalFrameCommands& commands) {
  for (ImageAssetId id : commands.image_invalidations) {
    state.images.erase(id.value);
  }
  std::size_t uploaded_bytes = 0;
  for (const ImageAsset& image : commands.image_uploads) {
    auto texture = make_image_texture(state, image);
    if (!texture) return std::unexpected(texture.error());
    uploaded_bytes += image.bitmap.pixels.size();
    state.images.insert_or_assign(image.id.value, std::move(*texture));
  }
  return uploaded_bytes;
}

Result<void> metal_encode_image(
    id<MTLRenderCommandEncoder> encoder,
    MetalRendererState& state,
    const ImageDraw& image) {
  const auto found = state.images.find(image.asset.id.value);
  if (found == state.images.end() || found->second.texture == nil) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "Metal image draw references an unavailable texture"});
  }
  const MetalClipResolution clip =
      metal_resolve_clip(state.framebuffer_size, image.clip_rect, image.clip_stack);
  if (!clip.visible) return {};
  const auto vertices = metal_rect_vertices(
      image.bounds,
      image_tint(image),
      image.metadata,
      image.composition_stack,
      image_uv(image));
  const simd_float2 viewport{
      state.framebuffer_size.width, state.framebuffer_size.height};
  [encoder setScissorRect:clip.scissor];
  [encoder setRenderPipelineState:state.pipelines.image];
  [encoder setVertexBytes:vertices.data() length:sizeof(vertices) atIndex:0];
  [encoder setVertexBytes:&viewport length:sizeof(viewport) atIndex:1];
  [encoder setFragmentTexture:found->second.texture atIndex:0];
  [encoder setFragmentSamplerState:
               (image.sampling == ImageSamplingMode::nearest
                    ? state.pipelines.nearest_sampler
                    : state.pipelines.linear_sampler)
                           atIndex:0];
  [encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip
              vertexStart:0
              vertexCount:4];
  return {};
}

}  // namespace cgpui
