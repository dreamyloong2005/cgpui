#include "cgpui/renderer/renderer.hpp"

#include <array>

// This executable must link only cgpui.renderer, never renderer-vulkan.
// Legacy entry points remain source-compatible but have neutral ownership.
int main() {
  const cgpui::ImageAsset asset{
      .id = {7}, .bitmap = {.width = 1, .height = 1, .stride = 4,
                           .pixels = {32, 64, 96, 255}}};
  const auto images = cgpui::vulkan_plan_image_uploads({&asset, 1});
  if (images.size() != 1 || images[0].rgba != asset.bitmap.pixels ||
      images[0].uploads[0].byte_size != 4) return 1;

  const cgpui::GlyphUploadRecord glyph{
      .page_index = 2, .width = 1, .height = 1, .stride = 1, .alpha = {200}};
  const cgpui::GlyphAtlasPage page{.page_index = 2, .size = {128, 128}};
  const auto glyphs = cgpui::vulkan_plan_glyph_atlas_uploads({&glyph, 1}, {&page, 1});
  if (glyphs.size() != 1 || glyphs[0].alpha != glyph.alpha ||
      glyphs[0].image.size.width != 128) return 2;
  cgpui::GlyphAtlasTextureResourceState textures;
  if (cgpui::vulkan_plan_glyph_atlas_dirty_uploads(textures, glyphs).size() != 1 ||
      !cgpui::vulkan_plan_glyph_atlas_dirty_uploads(textures, glyphs).empty() ||
      cgpui::vulkan_update_glyph_atlas_texture_resources(textures, {}).dropped_count != 1) return 3;
  cgpui::GlyphAtlasProductionResourceState production;
  if (cgpui::vulkan_plan_glyph_atlas_production_resources(production, glyphs).created_count != 1)
    return 4;

  cgpui::GlyphCache cache;
  cgpui::vulkan_consume_text_draw({}, cache);
  if (!cgpui::vulkan_build_textured_glyph_quads({}, cache).empty()) return 5;
  const cgpui::RoundedRectDraw rounded{.radius = {.top_left = 4}};
  if (cgpui::vulkan_tessellate_rounded_rects({&rounded, 1})[0].vertex_count != 8) return 6;
  const cgpui::TextSelectionDraw selection{};
  const cgpui::TextCaretDraw caret{};
  if (cgpui::vulkan_build_text_selection_geometry({&selection, 1}).size() != 1 ||
      cgpui::vulkan_build_text_caret_geometry({&caret, 1}).size() != 1) return 7;

  const std::array<cgpui::SolidRect, 2> rects{};
  if (cgpui::vulkan_build_renderer_command_report(rects, {}, cache).supported_command_count != 2 ||
      cgpui::vulkan_build_renderer_command_report(rects, {}, {}, cache).supported_command_count != 2 ||
      cgpui::vulkan_build_renderer_command_report(rects, {}, {}, {}, {}, cache).supported_command_count != 2 ||
      cgpui::vulkan_build_renderer_command_report(rects, {}, {}, {}, {}, {}, cache).supported_command_count != 2)
    return 8;
  const std::array<cgpui::RendererCommandStreamItem, 1> commands{{{
      .primitive_kind = cgpui::RendererPrimitiveKind::solid_rect}}};
  if (cgpui::vulkan_build_renderer_command_report(commands).supported_command_count != 1 ||
      cgpui::vulkan_build_renderer_frame_report(rects, {}, cache).renderer_batch_count != 1 ||
      cgpui::vulkan_build_renderer_frame_report(rects, {}, {}, {}, {}, cache).renderer_batch_count != 1)
    return 9;
  return cgpui::vulkan_build_renderer_command_batches(rects, {}).size() == 1 &&
         cgpui::vulkan_build_renderer_command_batches(rects, {}, {}).size() == 1 &&
         cgpui::vulkan_build_renderer_command_batches(rects, {}, {}, {}, {}).size() == 1 ? 0 : 10;
}
