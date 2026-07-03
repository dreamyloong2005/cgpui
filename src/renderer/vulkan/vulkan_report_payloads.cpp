#include "vulkan_report_internal.hpp"

#include <cstddef>

namespace cgpui {

void vulkan_record_renderer_text_statistics(
    RendererCommandReport& report,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache,
    std::span<std::vector<VulkanTextDrawAtlasPageUsage>>
        text_draw_atlas_pages) {
  for (std::size_t text_index = 0; text_index < text_draws.size();
       ++text_index) {
    const TextDraw& text_draw = text_draws[text_index];
    report.text_render.text_draw_count += 1;
    const std::size_t lookup_begin = glyph_cache.lookups().size();
    const std::size_t upload_begin = glyph_cache.upload_records().size();
    const std::vector<TexturedGlyphQuad> quads =
        vulkan_build_textured_glyph_quads(text_draw, glyph_cache);
    const std::size_t upload_count =
        glyph_cache.upload_records().size() - upload_begin;
    for (const TexturedGlyphQuad& quad : quads) {
      vulkan_append_text_draw_atlas_page_usage(
          text_draw_atlas_pages[text_index],
          quad.page_index);
    }

    for (std::size_t index = lookup_begin; index < glyph_cache.lookups().size();
         ++index) {
      if (glyph_cache.lookups()[index].hit) {
        report.text_render.glyph_cache_hit_count += 1;
      }
    }

    report.text_render.rasterized_glyph_count += upload_count;
    report.text_render.glyph_upload_record_count += upload_count;
    report.text_render.textured_glyph_quad_count += quads.size();
    if (quads.empty()) {
      report.text_render.metadata_only_text_draw_count += 1;
    } else {
      report.text_render.glyph_backed_text_draw_count += 1;
      report.text_render.text_sampler_pipeline_descriptor_count = 1;
      if (report.text_render.text_sampler_pipeline.ready()) {
        report.text_render.text_sampler_pipeline_ready_text_draw_count += 1;
      } else {
        report.text_render.text_sampler_pipeline_pending_text_draw_count += 1;
      }
    }
  }
}

void vulkan_record_renderer_image_statistics(
    RendererCommandReport& report,
    std::span<const ImageDraw> image_draws) {
  for (const ImageDraw& image_draw : image_draws) {
    report.image_render.image_draw_count += 1;
    report.image_render.image_upload_plan_count += 1;
    report.image_render.image_upload_byte_count += image_draw.asset.byte_size;
  }
}

} // namespace cgpui
