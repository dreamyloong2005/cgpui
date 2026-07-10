#pragma once

#include "cgpui/core/error.hpp"
#include "cgpui/renderer/glyph_atlas.hpp"
#include "cgpui/renderer/renderer_frame_diagnostic_snapshot.hpp"

#include <memory>
#include <span>
#include <vector>

namespace cgpui {

class RenderFrame {
 public:
  virtual ~RenderFrame() = default;
  virtual void clear(Color color) = 0;
  virtual void draw_rect(const SolidRect& rect) = 0;
  virtual void draw_rounded_rect(const RoundedRectDraw& rect) { (void)rect; }
  virtual void draw_text(const TextDraw& text) { (void)text; }
  virtual void draw_text_selection(const TextSelectionDraw& selection) {
    (void)selection;
  }
  virtual void draw_text_caret(const TextCaretDraw& caret) { (void)caret; }
  virtual void upload_image(const ImageAsset& image) { (void)image; }
  virtual void invalidate_image(ImageAssetId asset_id) { (void)asset_id; }
  virtual void draw_image(const ImageDraw& image) { (void)image; }
  virtual Result<void> present() = 0;
};

class Renderer {
 public:
  virtual ~Renderer() = default;

  virtual Result<void> resize(Size framebuffer_size, DpiScale scale) = 0;
  virtual Result<std::unique_ptr<RenderFrame>> begin_frame() = 0;
  [[nodiscard]] virtual const RendererFrameDiagnosticSnapshot*
  last_frame_diagnostic_snapshot() const;
};

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor& descriptor);
void vulkan_consume_text_draw(const TextDraw& text, GlyphCache& glyph_cache);
std::vector<GlyphAtlasUploadBatch> vulkan_plan_glyph_atlas_uploads(
    std::span<const GlyphUploadRecord> upload_records,
    std::span<const GlyphAtlasPage> atlas_pages);
std::vector<ImageUploadBatch> vulkan_plan_image_uploads(
    std::span<const ImageAsset> assets);
GlyphAtlasTextureResourcePlan vulkan_update_glyph_atlas_texture_resources(
    GlyphAtlasTextureResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches);
std::vector<GlyphAtlasDirtyUploadRange> vulkan_plan_glyph_atlas_dirty_uploads(
    GlyphAtlasTextureResourceState& state,
    std::span<const GlyphAtlasUploadBatch> upload_batches);
std::vector<TexturedGlyphQuad> vulkan_build_textured_glyph_quads(
    const TextDraw& text,
    GlyphCache& glyph_cache);
std::vector<RoundedRectTessellationRecord> vulkan_tessellate_rounded_rects(
    std::span<const RoundedRectDraw> rounded_rects);
std::vector<TextSelectionGeometryRecord> vulkan_build_text_selection_geometry(
    std::span<const TextSelectionDraw> selections);
std::vector<TextCaretGeometryRecord> vulkan_build_text_caret_geometry(
    std::span<const TextCaretDraw> carets);
RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const RendererCommandStreamItem> commands);
RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache);
RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache);
RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    GlyphCache& glyph_cache);
RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    std::span<const ImageDraw> image_draws,
    GlyphCache& glyph_cache);
RendererFrameReport vulkan_build_renderer_frame_report(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache);
RendererFrameReport vulkan_build_renderer_frame_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    GlyphCache& glyph_cache);
std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws);
std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws);
std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets);

} // namespace cgpui
