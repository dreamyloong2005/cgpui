#include "cgpui/renderer/planning.hpp"

#include <array>

namespace {
bool same_rect(cgpui::Rect lhs, cgpui::Rect rhs) {
  return lhs.origin.x == rhs.origin.x && lhs.origin.y == rhs.origin.y &&
         lhs.size.width == rhs.size.width && lhs.size.height == rhs.size.height;
}
bool cached_glyph_geometry_is_shared() {
  const cgpui::GlyphAtlasKey key{
      .byte_length = 1, .text = "A", .raster_advance = 6};
  cgpui::GlyphCache cache;
  cache.store({.key = key, .page_index = 0,
               .atlas_bounds = {{8, 16}, {2, 3}},
               .left_bearing = 1, .top_bearing = -2});
  auto legacy_cache = cache;
  const cgpui::TextDraw text{
      .glyphs = {{.key = key, .device_origin = {5, 7}, .device_advance = 6}}};
  const auto quads = cgpui::build_textured_glyph_quads(text, cache);
  const auto legacy = cgpui::vulkan_build_textured_glyph_quads(text, legacy_cache);
  return quads.size() == 1 && legacy.size() == 1 && cache.upload_records().empty() &&
         cache.lookups().back().hit && quads[0].device_bounds.origin.x == 6 &&
         quads[0].device_bounds.origin.y == 5 &&
         quads[0].atlas_uv_bounds.size.width == 2.0F / 256.0F &&
         same_rect(quads[0].device_bounds, legacy[0].device_bounds) &&
         same_rect(quads[0].atlas_uv_bounds, legacy[0].atlas_uv_bounds);
}
}

// This executable must link only cgpui.renderer, never renderer-vulkan.
// Native-backend-independent entry points exercise the same CPU algorithms.
int main() {
  if (!cached_glyph_geometry_is_shared()) return 11;
  const cgpui::ImageAsset asset{
      .id = {7}, .bitmap = {.width = 1, .height = 1, .stride = 4,
                           .pixels = {32, 64, 96, 255}}};
  const auto images = cgpui::plan_image_uploads({&asset, 1});
  if (images.size() != 1 || images[0].rgba != asset.bitmap.pixels ||
      images[0].uploads[0].byte_size != 4) return 1;

  const cgpui::GlyphUploadRecord glyph{
      .page_index = 2, .width = 1, .height = 1, .stride = 1, .alpha = {200}};
  const cgpui::GlyphAtlasPage page{.page_index = 2, .size = {128, 128}};
  const auto glyphs = cgpui::plan_glyph_atlas_uploads({&glyph, 1}, {&page, 1});
  if (glyphs.size() != 1 || glyphs[0].alpha != glyph.alpha ||
      glyphs[0].image.size.width != 128) return 2;
  cgpui::GlyphAtlasTextureResourceState textures;
  if (cgpui::plan_glyph_atlas_dirty_uploads(textures, glyphs).size() != 1 ||
      !cgpui::plan_glyph_atlas_dirty_uploads(textures, glyphs).empty() ||
      cgpui::update_glyph_atlas_texture_resources(textures, {}).dropped_count != 1) return 3;
  cgpui::GlyphAtlasProductionResourceState production;
  if (cgpui::plan_glyph_atlas_production_resources(production, glyphs).created_count != 1)
    return 4;

  cgpui::GlyphCache cache;
  cgpui::consume_text_draw({}, cache);
  if (!cgpui::build_textured_glyph_quads({}, cache).empty()) return 5;
  const cgpui::RoundedRectDraw rounded{.radius = {.top_left = 4}};
  if (cgpui::tessellate_rounded_rects({&rounded, 1})[0].vertex_count != 8) return 6;
  const cgpui::TextSelectionDraw selection{};
  const cgpui::TextCaretDraw caret{};
  if (cgpui::build_text_selection_geometry({&selection, 1}).size() != 1 ||
      cgpui::build_text_caret_geometry({&caret, 1}).size() != 1) return 7;

  const std::array<cgpui::SolidRect, 2> rects{};
  if (cgpui::build_renderer_command_report(rects, {}, cache).supported_command_count != 2 ||
      cgpui::build_renderer_command_report(rects, {}, {}, cache).supported_command_count != 2 ||
      cgpui::build_renderer_command_report(rects, {}, {}, {}, {}, cache).supported_command_count != 2 ||
      cgpui::build_renderer_command_report(rects, {}, {}, {}, {}, {}, cache).supported_command_count != 2)
    return 8;
  const std::array<cgpui::RendererCommandStreamItem, 1> commands{{{
      .primitive_kind = cgpui::RendererPrimitiveKind::solid_rect}}};
  if (cgpui::build_renderer_command_report(commands).supported_command_count != 1 ||
      cgpui::build_renderer_frame_report(rects, {}, cache).renderer_batch_count != 1 ||
      cgpui::build_renderer_frame_report(rects, {}, {}, {}, {}, cache).renderer_batch_count != 1)
    return 9;
  return cgpui::build_renderer_command_batches(rects, {}).size() == 1 &&
         cgpui::build_renderer_command_batches(rects, {}, {}).size() == 1 &&
         cgpui::build_renderer_command_batches(rects, {}, {}, {}, {}).size() == 1 ? 0 : 10;
}
