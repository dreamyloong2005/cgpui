#include "cgpui/renderer/renderer.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <algorithm>
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
          .primitive_kind = cgpui::RendererPrimitiveKind::text_caret,
          .command_index = 3,
          .clip_rect = clip,
          .metadata = metadata,
      },
  };

  const cgpui::RendererCommandReport report =
      cgpui::vulkan_build_renderer_command_report(commands);
  if (report.supported_command_count != 2 ||
      report.unsupported_command_count != 2 ||
      report.command_count() != commands.size()) {
    return 18;
  }
  if (report.batches.size() != 2 ||
      report.batches[0].key.primitive_kind !=
          cgpui::RendererPrimitiveKind::solid_rect ||
      report.batches[0].command_indices[0] != 0 ||
      report.batches[1].key.primitive_kind != cgpui::RendererPrimitiveKind::text ||
      report.batches[1].command_indices[0] != 2) {
    return 19;
  }
  if (report.unsupported_commands.size() != 2 ||
      report.unsupported_commands[0].primitive_kind !=
          cgpui::RendererPrimitiveKind::rounded_rect ||
      report.unsupported_commands[0].command_index != 1 ||
      report.unsupported_commands[0].reason !=
          cgpui::RendererUnsupportedCommandReason::unsupported_primitive ||
      report.unsupported_commands[0].message.find("rounded_rect") ==
          std::string::npos) {
    return 20;
  }

  return report.unsupported_commands[1].primitive_kind ==
                 cgpui::RendererPrimitiveKind::text_caret &&
                 report.unsupported_commands[1].command_index == 3 &&
                 report.unsupported_commands[1].message.find("text_caret") !=
                     std::string::npos
             ? 0
             : 21;
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
