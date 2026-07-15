#include "metal_renderer_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

MetalGlyphPageResource* find_page(
    MetalRendererState& state,
    std::size_t page_index) {
  const auto found = std::ranges::find_if(
      state.glyph_pages,
      [&](const MetalGlyphPageResource& page) {
        return page.page_index == page_index;
      });
  return found == state.glyph_pages.end() ? nullptr : &*found;
}

Result<MetalGlyphPageResource*> ensure_page(
    MetalRendererState& state,
    std::size_t page_index) {
  if (MetalGlyphPageResource* page = find_page(state, page_index)) return page;
  Size size{256.0F, 256.0F};
  for (const GlyphAtlasPage& page : state.glyph_cache.atlas_pages()) {
    if (page.page_index == page_index) size = page.size;
  }
  MTLTextureDescriptor* descriptor = [MTLTextureDescriptor
      texture2DDescriptorWithPixelFormat:MTLPixelFormatR8Unorm
                                   width:static_cast<NSUInteger>(size.width)
                                  height:static_cast<NSUInteger>(size.height)
                               mipmapped:NO];
  descriptor.usage = MTLTextureUsageShaderRead;
  descriptor.storageMode = MTLStorageModeShared;
  id<MTLTexture> texture = [state.device newTextureWithDescriptor:descriptor];
  if (texture == nil) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Metal glyph atlas texture creation failed"});
  }
  state.glyph_pages.push_back(
      MetalGlyphPageResource{.texture = texture, .page_index = page_index});
  return &state.glyph_pages.back();
}

Rect normalized_bounds(Rect bounds, id<MTLTexture> texture) {
  return Rect{
      .origin = {
          bounds.origin.x / static_cast<float>(texture.width),
          bounds.origin.y / static_cast<float>(texture.height)},
      .size = {
          bounds.size.width / static_cast<float>(texture.width),
          bounds.size.height / static_cast<float>(texture.height)}};
}

}  // namespace

Result<std::size_t> metal_encode_text(
    id<MTLRenderCommandEncoder> encoder,
    MetalRendererState& state,
    const TextDraw& text) {
  const MetalClipResolution clip =
      metal_resolve_clip(state.framebuffer_size, text.clip_rect, text.clip_stack);
  if (!clip.visible) return std::size_t{0};
  [encoder setScissorRect:clip.scissor];
  [encoder setRenderPipelineState:state.pipelines.glyph];
  [encoder setFragmentSamplerState:state.pipelines.linear_sampler atIndex:0];
  const simd_float2 viewport{
      state.framebuffer_size.width, state.framebuffer_size.height};
  [encoder setVertexBytes:&viewport length:sizeof(viewport) atIndex:1];

  std::size_t uploaded_bytes = 0;
  for (const TextGlyphPaint& glyph : text.glyphs) {
    GlyphCacheLookup lookup = state.glyph_cache.lookup(glyph.key);
    GlyphAtlasEntry entry;
    RasterizedGlyph rasterized;
    bool created = false;
    if (lookup.hit && lookup.entry) {
      entry = *lookup.entry;
    } else {
      rasterized = rasterize_fallback_glyph(glyph);
      const GlyphAtlasAllocation allocation =
          state.glyph_cache.allocate(rasterized);
      created = allocation.created;
      entry = GlyphAtlasEntry{
          .key = allocation.key,
          .page_index = allocation.page_index,
          .atlas_bounds = allocation.atlas_bounds,
          .advance = glyph.device_advance};
    }
    auto page = ensure_page(state, entry.page_index);
    if (!page) return std::unexpected(page.error());
    if (created) {
      [(*page)->texture
          replaceRegion:MTLRegionMake2D(
                            static_cast<NSUInteger>(entry.atlas_bounds.origin.x),
                            static_cast<NSUInteger>(entry.atlas_bounds.origin.y),
                            rasterized.bitmap.width,
                            rasterized.bitmap.height)
          mipmapLevel:0
          withBytes:rasterized.bitmap.alpha.data()
          bytesPerRow:rasterized.bitmap.stride];
      uploaded_bytes += rasterized.bitmap.alpha.size();
    }
    const Rect bounds{
        .origin = glyph.device_origin,
        .size = entry.atlas_bounds.size};
    const auto vertices = metal_rect_vertices(
        bounds,
        text.color,
        text.metadata,
        text.composition_stack,
        normalized_bounds(entry.atlas_bounds, (*page)->texture));
    [encoder setVertexBytes:vertices.data() length:sizeof(vertices) atIndex:0];
    [encoder setFragmentTexture:(*page)->texture atIndex:0];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip
                vertexStart:0
                vertexCount:4];
  }
  return uploaded_bytes;
}

}  // namespace cgpui
