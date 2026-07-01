#pragma once

#include "cgpui/core/error.hpp"
#include "cgpui/core/geometry.hpp"
#include "cgpui/platform/native_surface.hpp"
#include "cgpui/platform/target.hpp"
#include "cgpui/ui/text.hpp"

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

struct TextDraw {
  Rect bounds;
  Color color;
  FontDescriptor font;
  std::string content;
  std::size_t byte_length = 0;
  float font_size = 16.0F;
  std::vector<TextGlyphPaint> glyphs;
  std::optional<Rect> clip_rect;
  PaintMetadata metadata;
};

enum class RendererPrimitiveKind {
  solid_rect,
  text,
};

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

struct GlyphAtlasEntry {
  GlyphAtlasKey key;
  Rect atlas_bounds;
  float advance = 0.0F;

  friend bool operator==(
      const GlyphAtlasEntry&,
      const GlyphAtlasEntry&) = default;
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

 private:
  std::vector<GlyphAtlasEntry> entries_;
  std::vector<GlyphCacheRecord> lookups_;
};

class RenderFrame {
 public:
  virtual ~RenderFrame() = default;
  virtual void clear(Color color) = 0;
  virtual void draw_rect(const SolidRect& rect) = 0;
  virtual void draw_text(const TextDraw& text) { (void)text; }
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
std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws);

} // namespace cgpui
