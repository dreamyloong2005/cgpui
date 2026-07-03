#pragma once

#include "cgpui/renderer/glyph_atlas_types.hpp"
#include "cgpui/renderer/renderer_commands.hpp"

#include <cstddef>

namespace cgpui {

struct TextSamplerPipelineDescriptor {
  RendererPrimitiveKind primitive_kind = RendererPrimitiveKind::text;
  GlyphAtlasImageFormat sampled_image_format =
      GlyphAtlasImageFormat::alpha8_unorm;
  bool uses_alpha_sampling = true;
  bool uses_text_color = true;
  bool shader_modules_ready = false;
  bool descriptor_set_layout_ready = false;
  bool pipeline_layout_ready = false;
  bool graphics_pipeline_ready = false;

  [[nodiscard]] constexpr bool ready() const {
    return shader_modules_ready && descriptor_set_layout_ready &&
           pipeline_layout_ready && graphics_pipeline_ready;
  }

  friend bool operator==(
      const TextSamplerPipelineDescriptor&,
      const TextSamplerPipelineDescriptor&) = default;
};

struct RendererTextRenderReport {
  std::size_t text_draw_count = 0;
  std::size_t glyph_backed_text_draw_count = 0;
  std::size_t metadata_only_text_draw_count = 0;
  std::size_t glyph_cache_hit_count = 0;
  std::size_t rasterized_glyph_count = 0;
  std::size_t glyph_upload_record_count = 0;
  std::size_t textured_glyph_quad_count = 0;
  TextSamplerPipelineDescriptor text_sampler_pipeline;
  std::size_t text_sampler_pipeline_descriptor_count = 0;
  std::size_t text_sampler_pipeline_ready_text_draw_count = 0;
  std::size_t text_sampler_pipeline_pending_text_draw_count = 0;
};

} // namespace cgpui
