#include "cgpui/renderer/renderer.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>

namespace {

constexpr int kClientWidth = 160;
constexpr int kClientHeight = 120;

void pump_messages() {
  MSG message{};
  while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0) {
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }
}

class VisibleWindow {
 public:
  VisibleWindow() : instance_(GetModuleHandleW(nullptr)) {
    const wchar_t* class_name = L"CGPUIVulkanSolidRectTestWindow";

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.lpfnWndProc = DefWindowProcW;
    window_class.hInstance = instance_;
    window_class.lpszClassName = class_name;
    RegisterClassExW(&window_class);

    RECT window_rect{0, 0, kClientWidth, kClientHeight};
    AdjustWindowRectEx(&window_rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
    const int window_width = window_rect.right - window_rect.left;
    const int window_height = window_rect.bottom - window_rect.top;

    hwnd_ = CreateWindowExW(
        0,
        class_name,
        L"CGPUI Vulkan Solid Rect Test",
        WS_OVERLAPPEDWINDOW,
        64,
        64,
        window_width,
        window_height,
        nullptr,
        nullptr,
        instance_,
        nullptr);
    if (hwnd_ != nullptr) {
      ShowWindow(hwnd_, SW_SHOW);
      SetWindowPos(
          hwnd_,
          HWND_TOPMOST,
          64,
          64,
          window_width,
          window_height,
          SWP_SHOWWINDOW);
      UpdateWindow(hwnd_);
      SetForegroundWindow(hwnd_);
      pump_messages();
      Sleep(100);
    }
  }

  ~VisibleWindow() {
    if (hwnd_ != nullptr) {
      DestroyWindow(hwnd_);
    }
  }

  [[nodiscard]] bool valid() const {
    return instance_ != nullptr && hwnd_ != nullptr;
  }

  [[nodiscard]] cgpui::Win32SurfaceHandle surface() const {
    return cgpui::Win32SurfaceHandle{.hinstance = instance_, .hwnd = hwnd_};
  }

  [[nodiscard]] cgpui::Size framebuffer_size() const {
    RECT client_rect{};
    GetClientRect(hwnd_, &client_rect);
    return cgpui::Size{
        .width = static_cast<float>(client_rect.right - client_rect.left),
        .height = static_cast<float>(client_rect.bottom - client_rect.top),
    };
  }

  [[nodiscard]] bool sample_client_pixel(int x, int y, COLORREF& color) const {
    RECT client_rect{};
    if (GetClientRect(hwnd_, &client_rect) == 0) {
      return false;
    }

    const int client_width = static_cast<int>(client_rect.right - client_rect.left);
    const int client_height =
        static_cast<int>(client_rect.bottom - client_rect.top);
    POINT point{
        .x = std::clamp(x, 0, client_width - 1),
        .y = std::clamp(y, 0, client_height - 1),
    };
    if (ClientToScreen(hwnd_, &point) == 0) {
      return false;
    }

    HDC screen = GetDC(nullptr);
    if (screen == nullptr) {
      return false;
    }
    color = GetPixel(screen, point.x, point.y);
    ReleaseDC(nullptr, screen);
    return color != CLR_INVALID;
  }

 private:
  HINSTANCE instance_ = nullptr;
  HWND hwnd_ = nullptr;
};

bool is_blue_dominant(COLORREF color) {
  const auto red = static_cast<int>(GetRValue(color));
  const auto green = static_cast<int>(GetGValue(color));
  const auto blue = static_cast<int>(GetBValue(color));
  return blue >= green + 25 && green >= red + 40;
}

bool is_dark_clear_color(COLORREF color) {
  const auto red = static_cast<int>(GetRValue(color));
  const auto green = static_cast<int>(GetGValue(color));
  const auto blue = static_cast<int>(GetBValue(color));
  return red >= 55 && red <= 110 && green >= 55 && green <= 115 &&
         blue >= 55 && blue <= 125 && blue <= green + 25 &&
         green <= red + 25;
}

int test_glyph_cache_records_lookup_miss_and_hit() {
  cgpui::GlyphCache cache;
  const cgpui::GlyphAtlasKey key{
      .font_family = "Inter",
      .font_size = 20.0F,
      .glyph_index = 1,
      .byte_offset = 1,
      .byte_length = 3,
  };

  const cgpui::GlyphCacheLookup miss = cache.lookup(key);
  if (miss.hit || miss.entry.has_value() || cache.lookup_count() != 1) {
    return 6;
  }
  if (cache.lookups()[0].key != key || cache.lookups()[0].hit) {
    return 7;
  }

  cache.store(cgpui::GlyphAtlasEntry{
      .key = key,
      .atlas_bounds =
          cgpui::Rect{
              .origin = {.x = 8.0F, .y = 16.0F},
              .size = {.width = 10.0F, .height = 20.0F},
          },
      .advance = 10.0F,
  });

  const cgpui::GlyphCacheLookup hit = cache.lookup(key);
  if (!hit.hit || !hit.entry.has_value() || cache.lookup_count() != 2) {
    return 8;
  }

  return hit.entry->atlas_bounds.origin.x == 8.0F &&
                 hit.entry->advance == 10.0F &&
                 cache.lookups()[1].hit &&
                 cache.entries().size() == 1
             ? 0
             : 9;
}

int test_glyph_cache_allocates_atlas_pages_and_upload_records() {
  const cgpui::TextShapeRun run =
      cgpui::shape_text("ab", cgpui::FontDescriptor{.family = "Inter"}, 20.0F);
  const std::vector<cgpui::TextGlyphPaint> glyphs =
      cgpui::text_glyph_paint_metadata(run);
  const cgpui::RasterizedGlyph first =
      cgpui::rasterize_fallback_glyph(glyphs[0]);
  const cgpui::RasterizedGlyph second =
      cgpui::rasterize_fallback_glyph(glyphs[1]);

  cgpui::GlyphCache cache;
  const cgpui::GlyphAtlasAllocation first_allocation =
      cache.allocate(first);
  if (!first_allocation.created || first_allocation.page_index != 0 ||
      first_allocation.atlas_bounds.origin.x != 0.0F ||
      first_allocation.atlas_bounds.origin.y != 0.0F ||
      first_allocation.atlas_bounds.size.width != 10.0F ||
      first_allocation.atlas_bounds.size.height != 20.0F ||
      cache.atlas_pages().size() != 1 ||
      cache.upload_records().size() != 1) {
    return 22;
  }
  if (cache.upload_records()[0].key != glyphs[0].key ||
      cache.upload_records()[0].page_index != 0 ||
      cache.upload_records()[0].width != first.bitmap.width ||
      cache.upload_records()[0].height != first.bitmap.height ||
      cache.upload_records()[0].stride != first.bitmap.stride ||
      cache.upload_records()[0].alpha != first.bitmap.alpha) {
    return 23;
  }

  const cgpui::GlyphAtlasAllocation second_allocation =
      cache.allocate(second);
  if (!second_allocation.created || second_allocation.page_index != 0 ||
      second_allocation.atlas_bounds.origin.x != 10.0F ||
      second_allocation.atlas_bounds.origin.y != 0.0F ||
      cache.entries().size() != 2 ||
      cache.atlas_pages()[0].entries.size() != 2 ||
      cache.upload_records().size() != 2) {
    return 24;
  }

  const cgpui::GlyphAtlasAllocation repeated_allocation =
      cache.allocate(first);
  return !repeated_allocation.created &&
                 repeated_allocation.page_index == 0 &&
                 repeated_allocation.atlas_bounds.origin.x == 0.0F &&
                 cache.entries().size() == 2 &&
                 cache.upload_records().size() == 2
             ? 0
             : 25;
}

int test_glyph_upload_records_build_atlas_image_batches() {
  const cgpui::TextShapeRun run =
      cgpui::shape_text("ab", cgpui::FontDescriptor{.family = "Inter"}, 20.0F);
  const std::vector<cgpui::TextGlyphPaint> glyphs =
      cgpui::text_glyph_paint_metadata(run);

  cgpui::GlyphCache cache;
  const cgpui::RasterizedGlyph first =
      cgpui::rasterize_fallback_glyph(glyphs[0]);
  const cgpui::RasterizedGlyph second =
      cgpui::rasterize_fallback_glyph(glyphs[1]);
  (void)cache.allocate(first);
  (void)cache.allocate(second);

  const std::vector<cgpui::GlyphAtlasUploadBatch> batches =
      cgpui::vulkan_plan_glyph_atlas_uploads(
          cache.upload_records(),
          cache.atlas_pages());
  if (batches.size() != 1) {
    return 32;
  }

  const cgpui::GlyphAtlasUploadBatch& batch = batches[0];
  if (batch.image.page_index != 0 ||
      batch.image.format != cgpui::GlyphAtlasImageFormat::alpha8_unorm ||
      batch.image.size.width != 256.0F ||
      batch.image.size.height != 256.0F ||
      batch.uploads.size() != 2 ||
      batch.alpha.size() !=
          first.bitmap.alpha.size() + second.bitmap.alpha.size()) {
    return 33;
  }

  const cgpui::GlyphAtlasUploadRegion& first_upload = batch.uploads[0];
  if (first_upload.key != first.key || first_upload.page_index != 0 ||
      first_upload.atlas_bounds.origin.x != 0.0F ||
      first_upload.atlas_bounds.origin.y != 0.0F ||
      first_upload.width != first.bitmap.width ||
      first_upload.height != first.bitmap.height ||
      first_upload.stride != first.bitmap.stride ||
      first_upload.byte_offset != 0 ||
      first_upload.byte_size != first.bitmap.alpha.size()) {
    return 34;
  }

  const cgpui::GlyphAtlasUploadRegion& second_upload = batch.uploads[1];
  return second_upload.key == second.key && second_upload.page_index == 0 &&
                 second_upload.atlas_bounds.origin.x == 10.0F &&
                 second_upload.byte_offset == first.bitmap.alpha.size() &&
                 second_upload.byte_size == second.bitmap.alpha.size() &&
                 batch.image.upload_count == 2
             ? 0
             : 35;
}

int test_glyph_atlas_texture_resources_track_create_reuse_and_drop() {
  const cgpui::TextShapeRun run =
      cgpui::shape_text("ab", cgpui::FontDescriptor{.family = "Inter"}, 20.0F);
  const std::vector<cgpui::TextGlyphPaint> glyphs =
      cgpui::text_glyph_paint_metadata(run);

  cgpui::GlyphCache cache;
  (void)cache.allocate(cgpui::rasterize_fallback_glyph(glyphs[0]));
  (void)cache.allocate(cgpui::rasterize_fallback_glyph(glyphs[1]));

  const std::vector<cgpui::GlyphAtlasUploadBatch> batches =
      cgpui::vulkan_plan_glyph_atlas_uploads(
          cache.upload_records(),
          cache.atlas_pages());

  cgpui::GlyphAtlasTextureResourceState texture_resources;
  const cgpui::GlyphAtlasTextureResourcePlan first_plan =
      cgpui::vulkan_update_glyph_atlas_texture_resources(
          texture_resources,
          batches);
  if (first_plan.created_count != 1 || first_plan.reused_count != 0 ||
      first_plan.dropped_count != 0 ||
      first_plan.live_resources.size() != 1 ||
      texture_resources.live_resources().size() != 1) {
    return 36;
  }

  const cgpui::GlyphAtlasTextureResourceRecord& created =
      first_plan.live_resources[0];
  if (created.page_index != 0 ||
      created.status != cgpui::GlyphAtlasTextureResourceStatus::created ||
      created.image.size.width != 256.0F ||
      created.image.upload_count != 2 ||
      created.generation != 1) {
    return 37;
  }

  const cgpui::GlyphAtlasTextureResourcePlan second_plan =
      cgpui::vulkan_update_glyph_atlas_texture_resources(
          texture_resources,
          batches);
  if (second_plan.created_count != 0 || second_plan.reused_count != 1 ||
      second_plan.dropped_count != 0 ||
      second_plan.live_resources.size() != 1 ||
      second_plan.live_resources[0].status !=
          cgpui::GlyphAtlasTextureResourceStatus::reused ||
      second_plan.live_resources[0].generation != 1) {
    return 38;
  }

  const std::vector<cgpui::GlyphAtlasUploadBatch> empty_batches;
  const cgpui::GlyphAtlasTextureResourcePlan third_plan =
      cgpui::vulkan_update_glyph_atlas_texture_resources(
          texture_resources,
          empty_batches);
  return third_plan.created_count == 0 && third_plan.reused_count == 0 &&
                 third_plan.dropped_count == 1 &&
                 third_plan.dropped_resources.size() == 1 &&
                 third_plan.dropped_resources[0].page_index == 0 &&
                 third_plan.dropped_resources[0].status ==
                     cgpui::GlyphAtlasTextureResourceStatus::dropped &&
                 texture_resources.live_resources().empty()
             ? 0
             : 39;
}

int test_glyph_atlas_dirty_upload_ranges_only_include_new_regions() {
  const cgpui::TextShapeRun run =
      cgpui::shape_text("abc", cgpui::FontDescriptor{.family = "Inter"}, 20.0F);
  const std::vector<cgpui::TextGlyphPaint> glyphs =
      cgpui::text_glyph_paint_metadata(run);

  cgpui::GlyphCache cache;
  const cgpui::RasterizedGlyph first =
      cgpui::rasterize_fallback_glyph(glyphs[0]);
  const cgpui::RasterizedGlyph second =
      cgpui::rasterize_fallback_glyph(glyphs[1]);
  const cgpui::RasterizedGlyph third =
      cgpui::rasterize_fallback_glyph(glyphs[2]);
  (void)cache.allocate(first);
  (void)cache.allocate(second);

  cgpui::GlyphAtlasTextureResourceState texture_resources;
  const std::vector<cgpui::GlyphAtlasUploadBatch> first_batches =
      cgpui::vulkan_plan_glyph_atlas_uploads(
          cache.upload_records(),
          cache.atlas_pages());
  const std::vector<cgpui::GlyphAtlasDirtyUploadRange> first_dirty_ranges =
      cgpui::vulkan_plan_glyph_atlas_dirty_uploads(
          texture_resources,
          first_batches);
  if (first_dirty_ranges.size() != 1 ||
      first_dirty_ranges[0].page_index != 0 ||
      first_dirty_ranges[0].first_upload_index != 0 ||
      first_dirty_ranges[0].upload_count != 2 ||
      first_dirty_ranges[0].byte_offset != 0 ||
      first_dirty_ranges[0].byte_size !=
          first.bitmap.alpha.size() + second.bitmap.alpha.size()) {
    return 40;
  }

  const std::vector<cgpui::GlyphAtlasDirtyUploadRange> repeated_dirty_ranges =
      cgpui::vulkan_plan_glyph_atlas_dirty_uploads(
          texture_resources,
          first_batches);
  if (!repeated_dirty_ranges.empty()) {
    return 41;
  }

  (void)cache.allocate(third);
  const std::vector<cgpui::GlyphAtlasUploadBatch> second_batches =
      cgpui::vulkan_plan_glyph_atlas_uploads(
          cache.upload_records(),
          cache.atlas_pages());
  const std::vector<cgpui::GlyphAtlasDirtyUploadRange> second_dirty_ranges =
      cgpui::vulkan_plan_glyph_atlas_dirty_uploads(
          texture_resources,
          second_batches);
  return second_dirty_ranges.size() == 1 &&
                 second_dirty_ranges[0].page_index == 0 &&
                 second_dirty_ranges[0].first_upload_index == 2 &&
                 second_dirty_ranges[0].upload_count == 1 &&
                 second_dirty_ranges[0].byte_offset ==
                     first.bitmap.alpha.size() + second.bitmap.alpha.size() &&
                 second_dirty_ranges[0].byte_size ==
                     third.bitmap.alpha.size()
             ? 0
             : 42;
}

int test_text_draw_uses_glyph_cache_metadata() {
  cgpui::GlyphCache cache;
  cgpui::TextDraw text{
      .bounds =
          cgpui::Rect{
              .origin = {.x = 4.0F, .y = 6.0F},
              .size = {.width = 40.0F, .height = 20.0F},
          },
      .color = cgpui::Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
      .font = cgpui::FontDescriptor{.family = "Inter"},
      .content = "ab",
      .byte_length = 2,
      .font_size = 20.0F,
      .glyphs = cgpui::text_glyph_paint_metadata(
          cgpui::shape_text(
              "ab", cgpui::FontDescriptor{.family = "Inter"}, 20.0F),
          cgpui::Point{.x = 4.0F, .y = 6.0F}),
  };

  cgpui::vulkan_consume_text_draw(text, cache);
  if (cache.lookup_count() != 2 || cache.entries().size() != 2 ||
      cache.upload_records().size() != 2) {
    return 10;
  }
  if (cache.lookups()[0].hit || cache.lookups()[1].hit) {
    return 11;
  }

  cgpui::vulkan_consume_text_draw(text, cache);
  if (cache.lookup_count() != 4 || cache.entries().size() != 2 ||
      cache.upload_records().size() != 2) {
    return 12;
  }

  return cache.lookups()[2].hit && cache.lookups()[3].hit &&
                 cache.entries()[0].atlas_bounds.origin.x == 0.0F &&
                 cache.entries()[1].atlas_bounds.origin.x == 10.0F &&
                 cache.upload_records()[0].alpha.size() == 200
             ? 0
             : 13;
}

int test_text_draw_builds_textured_glyph_quads_from_atlas_entries() {
  cgpui::GlyphCache cache;
  const cgpui::Rect clip{
      .origin = {.x = 1.0F, .y = 2.0F},
      .size = {.width = 32.0F, .height = 18.0F},
  };
  const cgpui::PaintMetadata metadata{
      .opacity = 0.625F,
      .transform = cgpui::AffineTransform::translation(2.0F, 3.0F),
  };
  const cgpui::Color color{.r = 0.25F, .g = 0.5F, .b = 0.75F, .a = 0.8F};
  cgpui::TextDraw text{
      .bounds =
          cgpui::Rect{
              .origin = {.x = 4.0F, .y = 6.0F},
              .size = {.width = 40.0F, .height = 20.0F},
          },
      .color = color,
      .font = cgpui::FontDescriptor{.family = "Inter"},
      .content = "ab",
      .byte_length = 2,
      .font_size = 20.0F,
      .device_font_size = 20.0F,
      .glyphs = cgpui::text_glyph_paint_metadata(
          cgpui::shape_text(
              "ab", cgpui::FontDescriptor{.family = "Inter"}, 20.0F),
          cgpui::Point{.x = 4.0F, .y = 6.0F}),
      .clip_rect = clip,
      .metadata = metadata,
  };

  const std::vector<cgpui::TexturedGlyphQuad> quads =
      cgpui::vulkan_build_textured_glyph_quads(text, cache);
  if (quads.size() != 2 || cache.entries().size() != 2 ||
      cache.upload_records().size() != 2) {
    return 26;
  }

  const cgpui::TexturedGlyphQuad& first = quads[0];
  if (first.key != text.glyphs[0].key || first.page_index != 0 ||
      first.device_bounds.origin.x != 4.0F ||
      first.device_bounds.origin.y != 6.0F ||
      first.device_bounds.size.width != 10.0F ||
      first.device_bounds.size.height != 20.0F ||
      first.atlas_bounds.origin.x != 0.0F ||
      first.atlas_bounds.origin.y != 0.0F ||
      first.atlas_uv_bounds.origin.x != 0.0F ||
      first.atlas_uv_bounds.origin.y != 0.0F ||
      first.atlas_uv_bounds.size.width != 10.0F / 256.0F ||
      first.atlas_uv_bounds.size.height != 20.0F / 256.0F) {
    return 27;
  }
  if (first.color.r != color.r || first.color.g != color.g ||
      first.color.b != color.b || first.color.a != color.a ||
      !first.clip_rect.has_value() ||
      first.clip_rect->origin.x != clip.origin.x ||
      first.clip_rect->origin.y != clip.origin.y ||
      first.clip_rect->size.width != clip.size.width ||
      first.clip_rect->size.height != clip.size.height ||
      first.metadata != metadata) {
    return 28;
  }

  const cgpui::TexturedGlyphQuad& second = quads[1];
  return second.key == text.glyphs[1].key && second.page_index == 0 &&
                 second.device_bounds.origin.x == 14.0F &&
                 second.device_bounds.origin.y == 6.0F &&
                 second.atlas_bounds.origin.x == 10.0F &&
                 second.atlas_uv_bounds.origin.x == 10.0F / 256.0F &&
                 second.color.a == color.a && second.metadata == metadata
             ? 0
             : 29;
}

int test_renderer_batches_by_clip_opacity_transform_and_kind() {
  const cgpui::Rect clip{
      .origin = {.x = 2.0F, .y = 4.0F},
      .size = {.width = 40.0F, .height = 20.0F},
  };
  const cgpui::PaintMetadata faded_translated{
      .opacity = 0.5F,
      .transform = cgpui::AffineTransform::translation(3.0F, 7.0F),
  };
  const cgpui::PaintMetadata faded_scaled{
      .opacity = 0.5F,
      .transform = cgpui::AffineTransform::scale(2.0F, 2.0F),
  };

  const std::vector<cgpui::SolidRect> rects{
      cgpui::SolidRect{
          .rect = {.size = {.width = 10.0F, .height = 10.0F}},
          .color = {.r = 1.0F, .a = 1.0F},
          .clip_rect = clip,
          .metadata = faded_translated,
      },
      cgpui::SolidRect{
          .rect = {.origin = {.x = 12.0F}, .size = {.width = 10.0F, .height = 10.0F}},
          .color = {.g = 1.0F, .a = 1.0F},
          .clip_rect = clip,
          .metadata = faded_translated,
      },
      cgpui::SolidRect{
          .rect = {.origin = {.x = 24.0F}, .size = {.width = 10.0F, .height = 10.0F}},
          .color = {.b = 1.0F, .a = 1.0F},
          .clip_rect = clip,
          .metadata = faded_scaled,
      },
  };
  const std::vector<cgpui::TextDraw> text_draws{
      cgpui::TextDraw{
          .bounds = {.size = {.width = 20.0F, .height = 10.0F}},
          .color = {.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
          .font = {.family = "Inter"},
          .content = "hi",
          .byte_length = 2,
          .font_size = 12.0F,
          .clip_rect = clip,
          .metadata = faded_translated,
      },
  };

  const std::vector<cgpui::RendererCommandBatch> batches =
      cgpui::vulkan_build_renderer_command_batches(rects, text_draws);
  if (batches.size() != 3) {
    return 14;
  }

  if (batches[0].key.primitive_kind !=
          cgpui::RendererPrimitiveKind::solid_rect ||
      batches[0].command_count != 2 ||
      batches[0].command_indices.size() != 2 ||
      batches[0].command_indices[0] != 0 ||
      batches[0].command_indices[1] != 1 ||
      !batches[0].key.clip_rect.has_value() ||
      batches[0].key.clip_rect->origin.x != clip.origin.x ||
      batches[0].key.clip_rect->size.width != clip.size.width ||
      batches[0].key.metadata != faded_translated) {
    return 15;
  }

  if (batches[1].key.primitive_kind !=
          cgpui::RendererPrimitiveKind::solid_rect ||
      batches[1].command_count != 1 ||
      batches[1].command_indices[0] != 2 ||
      batches[1].key.metadata != faded_scaled) {
    return 16;
  }

  return batches[2].key.primitive_kind == cgpui::RendererPrimitiveKind::text &&
                 batches[2].command_count == 1 &&
                 batches[2].command_indices[0] == 0 &&
                 batches[2].key.clip_rect.has_value() &&
                 batches[2].key.metadata == faded_translated
             ? 0
             : 17;
}

int test_renderer_reports_unsupported_commands_without_dropping_supported() {
  const auto unknown_primitive = static_cast<cgpui::RendererPrimitiveKind>(999);
  const cgpui::PaintMetadata metadata{
      .opacity = 0.75F,
      .transform = cgpui::AffineTransform::translation(4.0F, 2.0F),
  };
  const cgpui::Rect clip{
      .origin = {.x = 1.0F, .y = 2.0F},
      .size = {.width = 30.0F, .height = 40.0F},
  };

  const std::vector<cgpui::RendererCommandStreamItem> commands{
      cgpui::RendererCommandStreamItem{
          .primitive_kind = cgpui::RendererPrimitiveKind::solid_rect,
          .command_index = 0,
          .clip_rect = clip,
          .metadata = metadata,
      },
      cgpui::RendererCommandStreamItem{
          .primitive_kind = cgpui::RendererPrimitiveKind::rounded_rect,
          .command_index = 1,
          .clip_rect = clip,
          .metadata = metadata,
      },
      cgpui::RendererCommandStreamItem{
          .primitive_kind = cgpui::RendererPrimitiveKind::text,
          .command_index = 2,
          .clip_rect = clip,
          .metadata = metadata,
      },
      cgpui::RendererCommandStreamItem{
          .primitive_kind = unknown_primitive,
          .command_index = 3,
          .clip_rect = clip,
          .metadata = metadata,
      },
  };

  const cgpui::RendererCommandReport report =
      cgpui::vulkan_build_renderer_command_report(commands);
  if (report.supported_command_count != 3 ||
      report.unsupported_command_count != 1 ||
      report.command_count() != commands.size()) {
    return 18;
  }
  if (report.batches.size() != 3 ||
      report.batches[0].key.primitive_kind !=
          cgpui::RendererPrimitiveKind::solid_rect ||
      report.batches[0].command_indices[0] != 0 ||
      report.batches[1].key.primitive_kind !=
          cgpui::RendererPrimitiveKind::rounded_rect ||
      report.batches[1].command_indices[0] != 1 ||
      report.batches[2].key.primitive_kind !=
          cgpui::RendererPrimitiveKind::text ||
      report.batches[2].command_indices[0] != 2) {
    return 19;
  }
  if (report.unsupported_commands.size() != 1 ||
      report.unsupported_commands[0].primitive_kind != unknown_primitive ||
      report.unsupported_commands[0].command_index != 3 ||
      report.unsupported_commands[0].reason !=
          cgpui::RendererUnsupportedCommandReason::unsupported_primitive ||
      report.unsupported_commands[0].message.find("unknown") ==
          std::string::npos) {
    return 20;
  }

  return 0;
}

int test_renderer_report_counts_textured_glyph_draw_preparation() {
  cgpui::GlyphCache cache;
  const std::vector<cgpui::SolidRect> rects{
      cgpui::SolidRect{
          .rect = {.size = {.width = 12.0F, .height = 8.0F}},
          .color = {.r = 0.1F, .g = 0.2F, .b = 0.3F, .a = 1.0F},
      },
  };
  const std::vector<cgpui::TextDraw> text_draws{
      cgpui::TextDraw{
          .bounds =
              cgpui::Rect{
                  .origin = {.x = 4.0F, .y = 6.0F},
                  .size = {.width = 40.0F, .height = 20.0F},
              },
          .color = {.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
          .font = {.family = "Inter"},
          .content = "ab",
          .byte_length = 2,
          .font_size = 20.0F,
          .device_font_size = 20.0F,
          .glyphs = cgpui::text_glyph_paint_metadata(
              cgpui::shape_text(
                  "ab", cgpui::FontDescriptor{.family = "Inter"}, 20.0F),
              cgpui::Point{.x = 4.0F, .y = 6.0F}),
      },
  };

  const cgpui::RendererCommandReport first_report =
      cgpui::vulkan_build_renderer_command_report(rects, text_draws, cache);
  if (first_report.supported_command_count != 2 ||
      first_report.text_render.text_draw_count != 1 ||
      first_report.text_render.glyph_backed_text_draw_count != 1 ||
      first_report.text_render.metadata_only_text_draw_count != 0 ||
      first_report.text_render.glyph_cache_hit_count != 0 ||
      first_report.text_render.rasterized_glyph_count != 2 ||
      first_report.text_render.glyph_upload_record_count != 2 ||
      first_report.text_render.textured_glyph_quad_count != 2) {
    return 30;
  }

  const cgpui::RendererCommandReport second_report =
      cgpui::vulkan_build_renderer_command_report(rects, text_draws, cache);
  return second_report.text_render.glyph_cache_hit_count == 2 &&
                 second_report.text_render.rasterized_glyph_count == 0 &&
                 second_report.text_render.glyph_upload_record_count == 0 &&
                 second_report.text_render.textured_glyph_quad_count == 2 &&
                 cache.upload_records().size() == 2
             ? 0
             : 31;
}

int test_renderer_report_distinguishes_text_sampler_pipeline_readiness() {
  cgpui::GlyphCache cache;
  const std::vector<cgpui::SolidRect> rects;
  const std::vector<cgpui::TextDraw> text_draws{
      cgpui::TextDraw{
          .bounds =
              cgpui::Rect{
                  .origin = {.x = 2.0F, .y = 3.0F},
                  .size = {.width = 30.0F, .height = 18.0F},
              },
          .color = {.r = 0.9F, .g = 0.8F, .b = 0.7F, .a = 1.0F},
          .font = {.family = "Inter"},
          .content = "ok",
          .byte_length = 2,
          .font_size = 18.0F,
          .device_font_size = 18.0F,
          .glyphs = cgpui::text_glyph_paint_metadata(
              cgpui::shape_text(
                  "ok", cgpui::FontDescriptor{.family = "Inter"}, 18.0F),
              cgpui::Point{.x = 2.0F, .y = 3.0F}),
      },
  };

  const cgpui::RendererCommandReport report =
      cgpui::vulkan_build_renderer_command_report(rects, text_draws, cache);
  if (report.text_render.textured_glyph_quad_count != 2 ||
      report.text_render.glyph_backed_text_draw_count != 1) {
    return 43;
  }

  const cgpui::TextSamplerPipelineDescriptor& descriptor =
      report.text_render.text_sampler_pipeline;
  if (descriptor.primitive_kind != cgpui::RendererPrimitiveKind::text ||
      descriptor.sampled_image_format !=
          cgpui::GlyphAtlasImageFormat::alpha8_unorm ||
      !descriptor.uses_alpha_sampling || !descriptor.uses_text_color ||
      descriptor.shader_modules_ready || descriptor.descriptor_set_layout_ready ||
      descriptor.pipeline_layout_ready || descriptor.graphics_pipeline_ready ||
      descriptor.ready()) {
    return 44;
  }

  return report.text_render.text_sampler_pipeline_descriptor_count == 1 &&
                 report.text_render.text_sampler_pipeline_ready_text_draw_count ==
                     0 &&
                 report.text_render
                         .text_sampler_pipeline_pending_text_draw_count == 1
             ? 0
             : 45;
}

int test_rounded_rects_build_tessellation_records_with_clip_metadata() {
  const cgpui::PaintMetadata metadata{
      .opacity = 0.625F,
      .transform = cgpui::AffineTransform::translation(5.0F, 7.0F),
  };
  const cgpui::Rect clip{
      .origin = {.x = 1.0F, .y = 2.0F},
      .size = {.width = 64.0F, .height = 48.0F},
  };
  const std::vector<cgpui::RoundedRectDraw> rounded_rects{
      cgpui::RoundedRectDraw{
          .rect =
              cgpui::Rect{
                  .origin = {.x = 8.0F, .y = 10.0F},
                  .size = {.width = 40.0F, .height = 20.0F},
              },
          .color = {.r = 0.2F, .g = 0.4F, .b = 0.6F, .a = 0.8F},
          .radius = cgpui::BorderRadii::corners(3.0F, 4.0F, 5.0F, 6.0F),
          .clip_rect = clip,
          .metadata = metadata,
      },
  };

  const std::vector<cgpui::RoundedRectTessellationRecord> records =
      cgpui::vulkan_tessellate_rounded_rects(rounded_rects);
  if (records.size() != 1) {
    return 46;
  }
  const cgpui::RoundedRectTessellationRecord& record = records[0];
  if (record.rect.origin.x != 8.0F || record.rect.size.width != 40.0F ||
      record.color.b != 0.6F || record.radius.top_left != 3.0F ||
      !record.clip_rect.has_value() ||
      record.clip_rect->size.width != clip.size.width ||
      record.metadata != metadata || record.corner_segment_count != 4 ||
      record.vertex_count != 20 || record.triangle_count != 18) {
    return 47;
  }

  cgpui::GlyphCache cache;
  const std::vector<cgpui::SolidRect> rects;
  const std::vector<cgpui::TextDraw> text_draws;
  const cgpui::RendererCommandReport report =
      cgpui::vulkan_build_renderer_command_report(
          rects,
          rounded_rects,
          text_draws,
          cache);
  return report.supported_command_count == 1 &&
                 report.unsupported_command_count == 0 &&
                 report.rounded_rect_tessellation_count == 1 &&
                 report.rounded_rect_tessellations.size() == 1 &&
                 report.rounded_rect_tessellations[0].radius.bottom_left ==
                     6.0F
             ? 0
             : 48;
}

int test_text_selection_and_caret_build_geometry_records_with_metadata() {
  const cgpui::PaintMetadata metadata{
      .opacity = 0.5F,
      .transform = cgpui::AffineTransform::translation(3.0F, 9.0F),
  };
  const cgpui::Rect clip{
      .origin = {.x = 2.0F, .y = 4.0F},
      .size = {.width = 80.0F, .height = 24.0F},
  };
  const std::vector<cgpui::TextSelectionDraw> selections{
      cgpui::TextSelectionDraw{
          .rect =
              cgpui::Rect{
                  .origin = {.x = 10.0F, .y = 12.0F},
                  .size = {.width = 32.0F, .height = 18.0F},
              },
          .color = {.r = 0.1F, .g = 0.3F, .b = 0.9F, .a = 0.6F},
          .range = {.start = 1, .end = 5, .collapsed = false},
          .font_size = 18.0F,
          .clip_rect = clip,
          .metadata = metadata,
      },
  };
  const std::vector<cgpui::TextCaretDraw> carets{
      cgpui::TextCaretDraw{
          .rect =
              cgpui::Rect{
                  .origin = {.x = 42.0F, .y = 12.0F},
                  .size = {.width = 1.0F, .height = 18.0F},
              },
          .color = {.r = 0.9F, .g = 0.9F, .b = 0.95F, .a = 1.0F},
          .byte_offset = 5,
          .font_size = 18.0F,
          .clip_rect = clip,
          .metadata = metadata,
      },
  };

  const std::vector<cgpui::TextSelectionGeometryRecord> selection_records =
      cgpui::vulkan_build_text_selection_geometry(selections);
  const std::vector<cgpui::TextCaretGeometryRecord> caret_records =
      cgpui::vulkan_build_text_caret_geometry(carets);
  if (selection_records.size() != 1 || caret_records.size() != 1) {
    return 49;
  }
  if (selection_records[0].rect.origin.x != 10.0F ||
      selection_records[0].range.start != 1 ||
      selection_records[0].range.end != 5 ||
      selection_records[0].range.collapsed ||
      selection_records[0].color.b != 0.9F ||
      !selection_records[0].clip_rect.has_value() ||
      selection_records[0].clip_rect->size.width != clip.size.width ||
      selection_records[0].metadata != metadata ||
      selection_records[0].vertex_count != 4 ||
      selection_records[0].triangle_count != 2) {
    return 50;
  }
  if (caret_records[0].rect.origin.x != 42.0F ||
      caret_records[0].byte_offset != 5 ||
      caret_records[0].color.a != 1.0F ||
      !caret_records[0].clip_rect.has_value() ||
      caret_records[0].metadata != metadata ||
      caret_records[0].vertex_count != 4 ||
      caret_records[0].triangle_count != 2) {
    return 51;
  }

  cgpui::GlyphCache cache;
  const std::vector<cgpui::SolidRect> rects;
  const std::vector<cgpui::RoundedRectDraw> rounded_rects;
  const std::vector<cgpui::TextDraw> text_draws;
  const cgpui::RendererCommandReport report =
      cgpui::vulkan_build_renderer_command_report(
          rects,
          rounded_rects,
          text_draws,
          selections,
          carets,
          cache);
  if (report.supported_command_count != 2 ||
      report.unsupported_command_count != 0 ||
      report.text_selection_geometry_count != 1 ||
      report.text_caret_geometry_count != 1 ||
      report.text_selection_geometries.size() != 1 ||
      report.text_caret_geometries.size() != 1) {
    return 52;
  }

  return report.batches.size() == 2 &&
                 report.batches[0].key.primitive_kind ==
                     cgpui::RendererPrimitiveKind::text_selection &&
                 report.batches[0].key.metadata == metadata &&
                 report.batches[0].command_indices[0] == 0 &&
                 report.batches[1].key.primitive_kind ==
                     cgpui::RendererPrimitiveKind::text_caret &&
                 report.batches[1].key.clip_rect.has_value() &&
                 report.batches[1].command_indices[0] == 0
             ? 0
             : 53;
}

int test_renderer_batches_distinguish_nested_clip_stack_metadata() {
  const cgpui::Rect outer_a{
      .origin = {.x = 0.0F, .y = 0.0F},
      .size = {.width = 100.0F, .height = 80.0F},
  };
  const cgpui::Rect outer_b{
      .origin = {.x = 4.0F, .y = 6.0F},
      .size = {.width = 100.0F, .height = 80.0F},
  };
  const cgpui::Rect inner{
      .origin = {.x = 12.0F, .y = 14.0F},
      .size = {.width = 32.0F, .height = 20.0F},
  };
  const std::vector<cgpui::SolidRect> rects{
      cgpui::SolidRect{
          .rect = {.origin = {.x = 16.0F, .y = 18.0F},
                   .size = {.width = 12.0F, .height = 8.0F}},
          .color = {.r = 0.2F, .g = 0.3F, .b = 0.4F, .a = 1.0F},
          .clip_rect = inner,
          .clip_stack =
              cgpui::RendererClipStackRecord{
                  .clips = {outer_a, inner},
                  .full_depth = 2,
                  .current_clip_rect = inner,
              },
      },
      cgpui::SolidRect{
          .rect = {.origin = {.x = 30.0F, .y = 18.0F},
                   .size = {.width = 12.0F, .height = 8.0F}},
          .color = {.r = 0.2F, .g = 0.3F, .b = 0.4F, .a = 1.0F},
          .clip_rect = inner,
          .clip_stack =
              cgpui::RendererClipStackRecord{
                  .clips = {outer_b, inner},
                  .full_depth = 2,
                  .current_clip_rect = inner,
              },
      },
  };
  cgpui::GlyphCache cache;
  const std::vector<cgpui::TextDraw> text_draws;
  const cgpui::RendererCommandReport report =
      cgpui::vulkan_build_renderer_command_report(rects, text_draws, cache);
  if (report.supported_command_count != 2 ||
      report.clip_stack_record_count != 2 ||
      report.max_clip_stack_depth != 2 ||
      report.batches.size() != 2) {
    return 54;
  }
  if (!report.batches[0].key.clip_rect.has_value() ||
      !report.batches[1].key.clip_rect.has_value() ||
      report.batches[0].key.clip_rect->origin.x != inner.origin.x ||
      report.batches[1].key.clip_rect->origin.x != inner.origin.x ||
      report.batches[0].key.clip_stack ==
          report.batches[1].key.clip_stack) {
    return 55;
  }
  return report.batches[0].key.clip_stack.clips[0].origin.x == 0.0F &&
                 report.batches[1].key.clip_stack.clips[0].origin.x == 4.0F
             ? 0
             : 56;
}

int test_renderer_reports_nested_opacity_transform_stack_metadata() {
  const cgpui::PaintMetadata first{
      .opacity = 0.5F,
      .transform = cgpui::AffineTransform::translation(2.0F, 3.0F),
  };
  const cgpui::PaintMetadata second{
      .opacity = 0.25F,
      .transform = cgpui::AffineTransform::translation(6.0F, 8.0F),
  };
  const cgpui::RendererCompositionStackRecord stack{
      .entries = {first, second},
      .full_depth = 2,
      .current_metadata = second,
  };
  const std::vector<cgpui::SolidRect> rects{
      cgpui::SolidRect{
          .rect = {.origin = {.x = 2.0F, .y = 4.0F},
                   .size = {.width = 20.0F, .height = 10.0F}},
          .color = {.r = 0.4F, .g = 0.5F, .b = 0.6F, .a = 1.0F},
          .composition_stack = stack,
          .metadata = second,
      },
  };
  const std::vector<cgpui::RoundedRectDraw> rounded_rects{
      cgpui::RoundedRectDraw{
          .rect = {.origin = {.x = 6.0F, .y = 8.0F},
                   .size = {.width = 18.0F, .height = 12.0F}},
          .color = {.r = 0.3F, .g = 0.2F, .b = 0.7F, .a = 1.0F},
          .radius = cgpui::BorderRadii::all(3.0F),
          .composition_stack = stack,
          .metadata = second,
      },
  };
  const std::vector<cgpui::TextDraw> text_draws{
      cgpui::TextDraw{
          .bounds = {.origin = {.x = 10.0F, .y = 12.0F},
                     .size = {.width = 24.0F, .height = 16.0F}},
          .color = {.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
          .font = {.family = "Inter"},
          .content = "hi",
          .byte_length = 2,
          .font_size = 16.0F,
          .composition_stack = stack,
          .metadata = second,
      },
  };
  const std::vector<cgpui::TextSelectionDraw> selections{
      cgpui::TextSelectionDraw{
          .rect = {.origin = {.x = 10.0F, .y = 12.0F},
                   .size = {.width = 18.0F, .height = 16.0F}},
          .color = {.r = 0.2F, .g = 0.4F, .b = 0.8F, .a = 0.5F},
          .range = {.start = 0, .end = 2, .collapsed = false},
          .font_size = 16.0F,
          .composition_stack = stack,
          .metadata = second,
      },
  };
  const std::vector<cgpui::TextCaretDraw> carets{
      cgpui::TextCaretDraw{
          .rect = {.origin = {.x = 28.0F, .y = 12.0F},
                   .size = {.width = 1.0F, .height = 16.0F}},
          .color = {.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
          .byte_offset = 2,
          .font_size = 16.0F,
          .composition_stack = stack,
          .metadata = second,
      },
  };

  cgpui::GlyphCache cache;
  const cgpui::RendererCommandReport report =
      cgpui::vulkan_build_renderer_command_report(
          rects,
          rounded_rects,
          text_draws,
          selections,
          carets,
          cache);
  if (report.supported_command_count != 5 ||
      report.composition_stack_record_count != 5 ||
      report.max_composition_stack_depth != 2 ||
      report.batches.size() != 5) {
    return 57;
  }
  if (report.batches[0].key.composition_stack.full_depth != 2 ||
      report.batches[0].key.composition_stack.current_metadata.opacity !=
          0.25F ||
      report.rounded_rect_tessellations[0]
              .composition_stack.current_metadata.opacity != 0.25F ||
      report.text_selection_geometries[0].composition_stack.full_depth != 2 ||
      report.text_caret_geometries[0].composition_stack.full_depth != 2) {
    return 58;
  }
  return report.batches[2].key.primitive_kind ==
                 cgpui::RendererPrimitiveKind::text &&
                 report.batches[4].key.primitive_kind ==
                     cgpui::RendererPrimitiveKind::text_caret
             ? 0
             : 59;
}

int test_renderer_report_builds_submission_plan_records() {
  const cgpui::Rect outer_clip{
      .origin = {.x = 0.0F, .y = 0.0F},
      .size = {.width = 128.0F, .height = 96.0F},
  };
  const cgpui::Rect inner_clip{
      .origin = {.x = 4.0F, .y = 6.0F},
      .size = {.width = 96.0F, .height = 64.0F},
  };
  const cgpui::RendererClipStackRecord clip_stack =
      cgpui::renderer_clip_stack_record(
          std::array<cgpui::Rect, 2>{outer_clip, inner_clip});

  const std::vector<cgpui::SolidRect> rects{
      cgpui::SolidRect{
          .rect = {.origin = {.x = 8.0F, .y = 10.0F},
                   .size = {.width = 24.0F, .height = 16.0F}},
          .color = {.r = 0.8F, .g = 0.2F, .b = 0.1F, .a = 1.0F},
          .clip_rect = inner_clip,
          .clip_stack = clip_stack,
      },
  };
  const std::vector<cgpui::RoundedRectDraw> rounded_rects{
      cgpui::RoundedRectDraw{
          .rect = {.origin = {.x = 36.0F, .y = 10.0F},
                   .size = {.width = 20.0F, .height = 16.0F}},
          .color = {.r = 0.1F, .g = 0.5F, .b = 0.8F, .a = 1.0F},
          .radius = cgpui::BorderRadii::all(4.0F),
          .clip_rect = inner_clip,
          .clip_stack = clip_stack,
      },
  };
  const std::vector<cgpui::TextDraw> text_draws{
      cgpui::TextDraw{
          .bounds = {.origin = {.x = 8.0F, .y = 32.0F},
                     .size = {.width = 48.0F, .height = 24.0F}},
          .color = {.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
          .font = {.family = "Inter"},
          .content = "ab",
          .byte_length = 2,
          .font_size = 20.0F,
          .device_font_size = 20.0F,
          .glyphs = cgpui::text_glyph_paint_metadata(
              cgpui::shape_text(
                  "ab", cgpui::FontDescriptor{.family = "Inter"}, 20.0F),
              cgpui::Point{.x = 8.0F, .y = 32.0F}),
          .clip_rect = inner_clip,
          .clip_stack = clip_stack,
      },
  };

  cgpui::GlyphCache cache;
  const cgpui::RendererCommandReport report =
      cgpui::vulkan_build_renderer_command_report(
          rects,
          rounded_rects,
          text_draws,
          cache);
  if (report.submission_plan_record_count != 3 ||
      report.submission_plan_records.size() != 3) {
    return 60;
  }

  const cgpui::RendererSubmissionPlanRecord& solid_submission =
      report.submission_plan_records[0];
  if (solid_submission.key.primitive_kind !=
          cgpui::RendererPrimitiveKind::solid_rect ||
      solid_submission.key.atlas_page_index.has_value() ||
      solid_submission.key.clip_stack.full_depth != 2 ||
      solid_submission.pipeline.primitive_kind !=
          cgpui::RendererPrimitiveKind::solid_rect ||
      solid_submission.command_count != 1 ||
      solid_submission.command_indices[0] != 0) {
    return 61;
  }

  const cgpui::RendererSubmissionPlanRecord& text_submission =
      report.submission_plan_records[2];
  if (text_submission.key.primitive_kind != cgpui::RendererPrimitiveKind::text ||
      !text_submission.key.atlas_page_index.has_value() ||
      *text_submission.key.atlas_page_index != 0 ||
      text_submission.key.clip_stack.full_depth != 2 ||
      text_submission.pipeline != report.text_render.text_sampler_pipeline ||
      text_submission.glyph_quad_count != 2 ||
      text_submission.command_count != 1 ||
      text_submission.command_indices[0] != 0) {
    return 62;
  }

  return report.batches.size() == 3 &&
                 report.batches[2].key.primitive_kind ==
                     cgpui::RendererPrimitiveKind::text
             ? 0
             : 63;
}

int test_renderer_frame_report_summarizes_frame_snapshot() {
  const std::vector<cgpui::SolidRect> rects{
      cgpui::SolidRect{
          .rect = {.origin = {.x = 2.0F, .y = 4.0F},
                   .size = {.width = 12.0F, .height = 8.0F}},
          .color = {.r = 0.2F, .g = 0.3F, .b = 0.4F, .a = 1.0F},
      },
  };
  const std::vector<cgpui::TextDraw> text_draws{
      cgpui::TextDraw{
          .bounds = {.origin = {.x = 6.0F, .y = 10.0F},
                     .size = {.width = 48.0F, .height = 24.0F}},
          .color = {.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
          .font = {.family = "Inter"},
          .content = "ab",
          .byte_length = 2,
          .font_size = 20.0F,
          .device_font_size = 20.0F,
          .glyphs = cgpui::text_glyph_paint_metadata(
              cgpui::shape_text(
                  "ab", cgpui::FontDescriptor{.family = "Inter"}, 20.0F),
              cgpui::Point{.x = 6.0F, .y = 10.0F}),
      },
  };

  cgpui::GlyphCache cache;
  const cgpui::RendererFrameReport frame_report =
      cgpui::vulkan_build_renderer_frame_report(rects, text_draws, cache);
  if (frame_report.command_report.supported_command_count != 2 ||
      frame_report.supported_primitive_count != 2 ||
      frame_report.renderer_batch_count != 2 ||
      frame_report.submission_plan_record_count != 2 ||
      frame_report.glyph_upload_record_count != 2 ||
      frame_report.textured_glyph_quad_count != 2) {
    return 64;
  }
  if (frame_report.gap_count != 1 ||
      frame_report.gaps[0].kind !=
          cgpui::RendererFrameGapKind::pending_text_sampler_pipeline ||
      frame_report.gaps[0].count != 1) {
    return 65;
  }

  const auto unknown_primitive = static_cast<cgpui::RendererPrimitiveKind>(999);
  const std::vector<cgpui::RendererCommandStreamItem> commands{
      cgpui::RendererCommandStreamItem{
          .primitive_kind = cgpui::RendererPrimitiveKind::solid_rect,
          .command_index = 0,
      },
      cgpui::RendererCommandStreamItem{
          .primitive_kind = unknown_primitive,
          .command_index = 1,
      },
  };
  const cgpui::RendererCommandReport command_report =
      cgpui::vulkan_build_renderer_command_report(commands);
  const cgpui::RendererFrameReport unsupported_frame =
      cgpui::renderer_frame_report_from_command_report(command_report);
  return unsupported_frame.unsupported_primitive_count == 1 &&
                 unsupported_frame.gap_count == 1 &&
                 unsupported_frame.gaps[0].kind ==
                     cgpui::RendererFrameGapKind::unsupported_command
             ? 0
             : 66;
}

} // namespace

int main() {
  if (const int result = test_glyph_cache_records_lookup_miss_and_hit();
      result != 0) {
    return result;
  }
  if (const int result =
          test_glyph_cache_allocates_atlas_pages_and_upload_records();
      result != 0) {
    return result;
  }
  if (const int result =
          test_glyph_upload_records_build_atlas_image_batches();
      result != 0) {
    return result;
  }
  if (const int result =
          test_glyph_atlas_texture_resources_track_create_reuse_and_drop();
      result != 0) {
    return result;
  }
  if (const int result =
          test_glyph_atlas_dirty_upload_ranges_only_include_new_regions();
      result != 0) {
    return result;
  }
  if (const int result = test_text_draw_uses_glyph_cache_metadata();
      result != 0) {
    return result;
  }
  if (const int result =
          test_text_draw_builds_textured_glyph_quads_from_atlas_entries();
      result != 0) {
    return result;
  }
  if (const int result =
          test_renderer_batches_by_clip_opacity_transform_and_kind();
      result != 0) {
    return result;
  }
  if (const int result =
          test_renderer_reports_unsupported_commands_without_dropping_supported();
      result != 0) {
    return result;
  }
  if (const int result =
          test_renderer_report_counts_textured_glyph_draw_preparation();
      result != 0) {
    return result;
  }
  if (const int result =
          test_renderer_report_distinguishes_text_sampler_pipeline_readiness();
      result != 0) {
    return result;
  }
  if (const int result =
          test_rounded_rects_build_tessellation_records_with_clip_metadata();
      result != 0) {
    return result;
  }
  if (const int result =
          test_text_selection_and_caret_build_geometry_records_with_metadata();
      result != 0) {
    return result;
  }
  if (const int result =
          test_renderer_batches_distinguish_nested_clip_stack_metadata();
      result != 0) {
    return result;
  }
  if (const int result =
          test_renderer_reports_nested_opacity_transform_stack_metadata();
      result != 0) {
    return result;
  }
  if (const int result = test_renderer_report_builds_submission_plan_records();
      result != 0) {
    return result;
  }
  if (const int result = test_renderer_frame_report_summarizes_frame_snapshot();
      result != 0) {
    return result;
  }

  SetProcessDPIAware();
  VisibleWindow window;
  if (!window.valid()) {
    return 4;
  }

  const cgpui::Size framebuffer_size = window.framebuffer_size();
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.surface(),
      .framebuffer_size = framebuffer_size,
      .scale = cgpui::DpiScale{1.0F}});
  if (!renderer) {
    std::cerr << renderer.error().message << '\n';
    return 1;
  }

  auto frame = (*renderer)->begin_frame();
  if (!frame) {
    std::cerr << frame.error().message << '\n';
    return 2;
  }

  (*frame)->clear(cgpui::Color{.r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F});
  (*frame)->draw_rect(cgpui::SolidRect{
      .rect =
          cgpui::Rect{
              .origin =
                  cgpui::Point{
                      .x = framebuffer_size.width * 0.10F,
                      .y = framebuffer_size.height * 0.10F,
                  },
              .size =
                  cgpui::Size{
                      .width = framebuffer_size.width * 0.80F,
                      .height = framebuffer_size.height * 0.80F,
                  },
          },
      .color = cgpui::Color{.r = 0.23F, .g = 0.55F, .b = 0.86F, .a = 1.0F},
      .clip_rect =
          cgpui::Rect{
              .origin =
                  cgpui::Point{
                      .x = framebuffer_size.width * 0.10F,
                      .y = framebuffer_size.height * 0.10F,
                  },
              .size =
                  cgpui::Size{
                      .width = framebuffer_size.width * 0.42F,
                      .height = framebuffer_size.height * 0.80F,
                  },
          },
  });

  if (auto presented = (*frame)->present(); !presented) {
    std::cerr << presented.error().message << '\n';
    return 3;
  }

  COLORREF sampled = CLR_INVALID;
  for (int attempt = 0; attempt < 10; ++attempt) {
    pump_messages();
    Sleep(50);
    if (window.sample_client_pixel(
            static_cast<int>(framebuffer_size.width * 0.30F),
            static_cast<int>(framebuffer_size.height * 0.50F),
            sampled) &&
        is_blue_dominant(sampled)) {
      COLORREF clipped_sample = CLR_INVALID;
      if (window.sample_client_pixel(
              static_cast<int>(framebuffer_size.width * 0.70F),
              static_cast<int>(framebuffer_size.height * 0.50F),
              clipped_sample) &&
          is_dark_clear_color(clipped_sample)) {
        return 0;
      }
      sampled = clipped_sample;
      break;
    }
  }

  if (sampled != CLR_INVALID) {
    std::cerr << "Expected clipped solid rect outside sample to stay clear, "
              << "got RGB(" << static_cast<int>(GetRValue(sampled)) << ", "
              << static_cast<int>(GetGValue(sampled)) << ", "
              << static_cast<int>(GetBValue(sampled)) << ")\n";
  }
  return 5;
}
