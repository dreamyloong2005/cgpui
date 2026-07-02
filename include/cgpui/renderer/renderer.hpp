#pragma once

#include "cgpui/core/error.hpp"
#include "cgpui/core/geometry.hpp"
#include "cgpui/platform/native_surface.hpp"
#include "cgpui/platform/target.hpp"
#include "cgpui/ui/text.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cgpui {

enum class RendererBackendTarget {
  vulkan,
  metal,
};

[[nodiscard]] constexpr std::string_view renderer_backend_target_name(
    RendererBackendTarget target) {
  switch (target) {
    case RendererBackendTarget::vulkan:
      return "Vulkan";
    case RendererBackendTarget::metal:
      return "Metal";
  }

  return "Unknown";
}

[[nodiscard]] constexpr RendererBackendTarget default_renderer_backend_for(
    DesktopPlatformTarget target) {
  switch (target) {
    case DesktopPlatformTarget::windows:
    case DesktopPlatformTarget::linux_wayland:
      return RendererBackendTarget::vulkan;
    case DesktopPlatformTarget::macos_cocoa:
      return RendererBackendTarget::metal;
  }

  return RendererBackendTarget::vulkan;
}

struct RenderSurfaceDescriptor {
  NativeSurfaceHandle native_surface;
  Size framebuffer_size;
  DpiScale scale;
};

struct PaintMetadata {
  float opacity = 1.0F;
  AffineTransform transform;

  friend bool operator==(
      const PaintMetadata&,
      const PaintMetadata&) = default;
};

struct SolidRect {
  Rect rect;
  Color color;
  std::optional<Rect> clip_rect;
  PaintMetadata metadata;
};

struct RoundedRectDraw {
  Rect rect;
  Color color;
  BorderRadii radius;
  std::optional<Rect> clip_rect;
  PaintMetadata metadata;
};

struct TextDraw {
  Rect bounds;
  Color color;
  FontDescriptor font;
  std::string content;
  std::size_t byte_length = 0;
  float font_size = 16.0F;
  DpiScale scale;
  float device_font_size = 16.0F;
  std::vector<TextGlyphPaint> glyphs;
  std::optional<Rect> clip_rect;
  PaintMetadata metadata;
};

struct TextSelectionDraw {
  Rect rect;
  Color color;
  TextSelectionRange range;
  float font_size = 16.0F;
  std::optional<Rect> clip_rect;
  PaintMetadata metadata;
};

struct TextCaretDraw {
  Rect rect;
  Color color;
  std::size_t byte_offset = 0;
  float font_size = 16.0F;
  std::optional<Rect> clip_rect;
  PaintMetadata metadata;
};

enum class RendererPrimitiveKind {
  solid_rect,
  rounded_rect,
  text,
  text_selection,
  text_caret,
};

[[nodiscard]] constexpr std::string_view renderer_primitive_kind_name(
    RendererPrimitiveKind kind) {
  switch (kind) {
    case RendererPrimitiveKind::solid_rect:
      return "solid_rect";
    case RendererPrimitiveKind::rounded_rect:
      return "rounded_rect";
    case RendererPrimitiveKind::text:
      return "text";
    case RendererPrimitiveKind::text_selection:
      return "text_selection";
    case RendererPrimitiveKind::text_caret:
      return "text_caret";
  }

  return "unknown";
}

struct RendererCommandBatchKey {
  RendererPrimitiveKind primitive_kind = RendererPrimitiveKind::solid_rect;
  std::optional<Rect> clip_rect;
  PaintMetadata metadata;
};

struct RendererCommandBatch {
  RendererCommandBatchKey key;
  std::size_t command_count = 0;
  std::vector<std::size_t> command_indices;
};

struct RendererCommandStreamItem {
  RendererPrimitiveKind primitive_kind = RendererPrimitiveKind::solid_rect;
  std::size_t command_index = 0;
  std::optional<Rect> clip_rect;
  PaintMetadata metadata;
};

enum class RendererUnsupportedCommandReason {
  unsupported_primitive,
};

struct RendererUnsupportedCommandDiagnostic {
  RendererPrimitiveKind primitive_kind = RendererPrimitiveKind::solid_rect;
  std::size_t command_index = 0;
  RendererUnsupportedCommandReason reason =
      RendererUnsupportedCommandReason::unsupported_primitive;
  std::string message;
};

enum class GlyphAtlasImageFormat {
  alpha8_unorm,
};

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

struct RoundedRectTessellationRecord {
  Rect rect;
  Color color;
  BorderRadii radius;
  std::optional<Rect> clip_rect;
  PaintMetadata metadata;
  std::size_t corner_segment_count = 0;
  std::size_t vertex_count = 0;
  std::size_t triangle_count = 0;
};

struct TextSelectionGeometryRecord {
  Rect rect;
  Color color;
  TextSelectionRange range;
  float font_size = 16.0F;
  std::optional<Rect> clip_rect;
  PaintMetadata metadata;
  std::size_t vertex_count = 0;
  std::size_t triangle_count = 0;
};

struct TextCaretGeometryRecord {
  Rect rect;
  Color color;
  std::size_t byte_offset = 0;
  float font_size = 16.0F;
  std::optional<Rect> clip_rect;
  PaintMetadata metadata;
  std::size_t vertex_count = 0;
  std::size_t triangle_count = 0;
};

struct RendererCommandReport {
  std::vector<RendererCommandBatch> batches;
  std::vector<RendererUnsupportedCommandDiagnostic> unsupported_commands;
  std::vector<RoundedRectTessellationRecord> rounded_rect_tessellations;
  std::vector<TextSelectionGeometryRecord> text_selection_geometries;
  std::vector<TextCaretGeometryRecord> text_caret_geometries;
  std::size_t supported_command_count = 0;
  std::size_t unsupported_command_count = 0;
  std::size_t rounded_rect_tessellation_count = 0;
  std::size_t text_selection_geometry_count = 0;
  std::size_t text_caret_geometry_count = 0;
  RendererTextRenderReport text_render;

  [[nodiscard]] std::size_t command_count() const {
    return supported_command_count + unsupported_command_count;
  }
};

struct GlyphAtlasEntry {
  GlyphAtlasKey key;
  std::size_t page_index = 0;
  Rect atlas_bounds;
  float advance = 0.0F;

  friend bool operator==(
      const GlyphAtlasEntry&,
      const GlyphAtlasEntry&) = default;
};

struct GlyphAtlasAllocation {
  GlyphAtlasKey key;
  std::size_t page_index = 0;
  Rect atlas_bounds;
  bool created = false;
};

struct GlyphUploadRecord {
  GlyphAtlasKey key;
  std::size_t page_index = 0;
  Rect atlas_bounds;
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::uint32_t stride = 0;
  std::vector<std::uint8_t> alpha;
};

struct GlyphAtlasImageDescriptor {
  std::size_t page_index = 0;
  Size size{.width = 256.0F, .height = 256.0F};
  GlyphAtlasImageFormat format = GlyphAtlasImageFormat::alpha8_unorm;
  std::size_t upload_count = 0;
};

struct GlyphAtlasUploadRegion {
  GlyphAtlasKey key;
  std::size_t page_index = 0;
  Rect atlas_bounds;
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::uint32_t stride = 0;
  std::size_t byte_offset = 0;
  std::size_t byte_size = 0;
};

struct GlyphAtlasUploadBatch {
  GlyphAtlasImageDescriptor image;
  std::vector<GlyphAtlasUploadRegion> uploads;
  std::vector<std::uint8_t> alpha;
};

enum class GlyphAtlasTextureResourceStatus {
  created,
  reused,
  dropped,
};

struct GlyphAtlasTextureResourceRecord {
  std::size_t page_index = 0;
  GlyphAtlasImageDescriptor image;
  GlyphAtlasTextureResourceStatus status =
      GlyphAtlasTextureResourceStatus::created;
  std::size_t generation = 0;
};

struct GlyphAtlasTextureResourcePlan {
  std::vector<GlyphAtlasTextureResourceRecord> live_resources;
  std::vector<GlyphAtlasTextureResourceRecord> dropped_resources;
  std::size_t created_count = 0;
  std::size_t reused_count = 0;
  std::size_t dropped_count = 0;
};

struct GlyphAtlasDirtyUploadRange {
  std::size_t page_index = 0;
  std::size_t first_upload_index = 0;
  std::size_t upload_count = 0;
  std::size_t byte_offset = 0;
  std::size_t byte_size = 0;
};

class GlyphAtlasTextureResourceState {
 public:
  [[nodiscard]] std::span<const GlyphAtlasTextureResourceRecord>
  live_resources() const {
    return resources_;
  }

 private:
  friend GlyphAtlasTextureResourcePlan
  vulkan_update_glyph_atlas_texture_resources(
      GlyphAtlasTextureResourceState& state,
      std::span<const GlyphAtlasUploadBatch> upload_batches);
  friend std::vector<GlyphAtlasDirtyUploadRange>
  vulkan_plan_glyph_atlas_dirty_uploads(
      GlyphAtlasTextureResourceState& state,
      std::span<const GlyphAtlasUploadBatch> upload_batches);

  std::vector<GlyphAtlasTextureResourceRecord> resources_;
  std::size_t next_generation_ = 1;
};

struct GlyphAtlasPage {
  std::size_t page_index = 0;
  Size size{.width = 256.0F, .height = 256.0F};
  Point cursor;
  float row_height = 0.0F;
  std::vector<GlyphAtlasEntry> entries;
};

struct TexturedGlyphQuad {
  GlyphAtlasKey key;
  std::size_t page_index = 0;
  Rect device_bounds;
  Rect atlas_bounds;
  Rect atlas_uv_bounds;
  Color color;
  std::optional<Rect> clip_rect;
  PaintMetadata metadata;
};

struct GlyphCacheRecord {
  GlyphAtlasKey key;
  bool hit = false;
};

struct GlyphCacheLookup {
  GlyphAtlasKey key;
  bool hit = false;
  std::optional<GlyphAtlasEntry> entry;
};

class GlyphCache {
 public:
  [[nodiscard]] GlyphCacheLookup lookup(const GlyphAtlasKey& key) {
    for (const GlyphAtlasEntry& entry : entries_) {
      if (entry.key == key) {
        lookups_.push_back(GlyphCacheRecord{.key = key, .hit = true});
        return GlyphCacheLookup{.key = key, .hit = true, .entry = entry};
      }
    }

    lookups_.push_back(GlyphCacheRecord{.key = key, .hit = false});
    return GlyphCacheLookup{.key = key};
  }

  [[nodiscard]] GlyphAtlasAllocation allocate(const RasterizedGlyph& glyph) {
    if (const GlyphAtlasEntry* existing = find_entry(glyph.key);
        existing != nullptr) {
      return GlyphAtlasAllocation{
          .key = glyph.key,
          .page_index = existing->page_index,
          .atlas_bounds = existing->atlas_bounds,
      };
    }

    const Size glyph_size{
        .width = static_cast<float>(glyph.bitmap.width),
        .height = static_cast<float>(glyph.bitmap.height),
    };
    if (pages_.empty()) {
      pages_.push_back(make_page(0, glyph_size));
    }

    Rect atlas_bounds;
    if (!allocate_from_page(pages_.back(), glyph_size, atlas_bounds)) {
      pages_.push_back(make_page(pages_.size(), glyph_size));
      (void)allocate_from_page(pages_.back(), glyph_size, atlas_bounds);
    }

    GlyphAtlasEntry entry{
        .key = glyph.key,
        .page_index = pages_.back().page_index,
        .atlas_bounds = atlas_bounds,
        .advance = glyph.advance,
    };
    pages_.back().entries.push_back(entry);
    entries_.push_back(entry);
    uploads_.push_back(GlyphUploadRecord{
        .key = glyph.key,
        .page_index = entry.page_index,
        .atlas_bounds = atlas_bounds,
        .width = glyph.bitmap.width,
        .height = glyph.bitmap.height,
        .stride = glyph.bitmap.stride,
        .alpha = glyph.bitmap.alpha,
    });

    return GlyphAtlasAllocation{
        .key = glyph.key,
        .page_index = entry.page_index,
        .atlas_bounds = atlas_bounds,
        .created = true,
    };
  }

  void store(GlyphAtlasEntry entry) {
    for (GlyphAtlasEntry& existing : entries_) {
      if (existing.key == entry.key) {
        existing = std::move(entry);
        return;
      }
    }
    entries_.push_back(std::move(entry));
  }

  [[nodiscard]] std::size_t lookup_count() const {
    return lookups_.size();
  }

  [[nodiscard]] std::span<const GlyphCacheRecord> lookups() const {
    return lookups_;
  }

  [[nodiscard]] std::span<const GlyphAtlasEntry> entries() const {
    return entries_;
  }

  [[nodiscard]] std::span<const GlyphAtlasPage> atlas_pages() const {
    return pages_;
  }

  [[nodiscard]] std::span<const GlyphUploadRecord> upload_records() const {
    return uploads_;
  }

 private:
  [[nodiscard]] const GlyphAtlasEntry* find_entry(
      const GlyphAtlasKey& key) const {
    for (const GlyphAtlasEntry& entry : entries_) {
      if (entry.key == key) {
        return &entry;
      }
    }
    return nullptr;
  }

  [[nodiscard]] static GlyphAtlasPage make_page(
      std::size_t page_index,
      Size minimum_size) {
    return GlyphAtlasPage{
        .page_index = page_index,
        .size =
            Size{
                .width = std::max(256.0F, minimum_size.width),
                .height = std::max(256.0F, minimum_size.height),
            },
    };
  }

  [[nodiscard]] static bool allocate_from_page(
      GlyphAtlasPage& page,
      Size glyph_size,
      Rect& atlas_bounds) {
    if (glyph_size.width <= 0.0F || glyph_size.height <= 0.0F ||
        glyph_size.width > page.size.width ||
        glyph_size.height > page.size.height) {
      return false;
    }

    if (page.cursor.x + glyph_size.width > page.size.width) {
      page.cursor.x = 0.0F;
      page.cursor.y += page.row_height;
      page.row_height = 0.0F;
    }
    if (page.cursor.y + glyph_size.height > page.size.height) {
      return false;
    }

    atlas_bounds = Rect{.origin = page.cursor, .size = glyph_size};
    page.cursor.x += glyph_size.width;
    page.row_height = std::max(page.row_height, glyph_size.height);
    return true;
  }

  std::vector<GlyphAtlasEntry> entries_;
  std::vector<GlyphCacheRecord> lookups_;
  std::vector<GlyphAtlasPage> pages_;
  std::vector<GlyphUploadRecord> uploads_;
};

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
  virtual Result<void> present() = 0;
};

class Renderer {
 public:
  virtual ~Renderer() = default;

  virtual Result<void> resize(Size framebuffer_size, DpiScale scale) = 0;
  virtual Result<std::unique_ptr<RenderFrame>> begin_frame() = 0;
};

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor& descriptor);
void vulkan_consume_text_draw(const TextDraw& text, GlyphCache& glyph_cache);
std::vector<GlyphAtlasUploadBatch> vulkan_plan_glyph_atlas_uploads(
    std::span<const GlyphUploadRecord> upload_records,
    std::span<const GlyphAtlasPage> atlas_pages);
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
