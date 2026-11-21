#include "metal_renderer_internal.hpp"

#include "../renderer_text_glyph_placement_internal.hpp"
#include "cgpui/ui/text_glyph_rasterizer.hpp"

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
    // As vulkan_build_textured_glyph_quads: native glyphs by glyph id with
    // the pen and baseline snapped, deterministic glyphs by cluster text.
    const bool native =
        glyph.key.shaping_backend != TextShapingBackend::deterministic_fallback;
    const GlyphPlacement placement = native
        ? native_glyph_placement(glyph)
        : cell_glyph_placement(glyph, text.content);
    const TextGlyphPaint& raster_glyph = placement.raster;
    GlyphCacheLookup lookup = state.glyph_cache.lookup(raster_glyph.key);
    GlyphAtlasEntry entry;
    RasterizedGlyph rasterized;
    bool created = false;
    if (lookup.hit && lookup.entry) {
      entry = *lookup.entry;
    } else {
      auto raster = rasterize_text_glyph(raster_glyph, raster_glyph.key.text);
      if (!raster) {
        state.glyph_cache.set_rasterization_error(raster.error());
        return std::unexpected(raster.error());
      }
      rasterized = std::move(*raster);
      const GlyphAtlasAllocation allocation =
          state.glyph_cache.allocate(rasterized);
      created = allocation.created;
      entry = GlyphAtlasEntry{
          .key = allocation.key,
          .page_index = allocation.page_index,
          .atlas_bounds = allocation.atlas_bounds,
          .advance = glyph.device_advance,
          .left_bearing = rasterized.left_bearing,
          .top_bearing = rasterized.top_bearing,
          .baseline = rasterized.baseline};
    }
    auto page = ensure_page(state, entry.page_index);
    if (!page) return std::unexpected(page.error());
    if (created && rasterized.bitmap.width > 0 && rasterized.bitmap.height > 0) {
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
    if (entry.atlas_bounds.size.width <= 0.0F ||
        entry.atlas_bounds.size.height <= 0.0F) {
      continue;
    }
    const Rect bounds{
        .origin = {placement.device_pen.x + entry.left_bearing,
                   placement.device_pen.y + entry.top_bearing},
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
