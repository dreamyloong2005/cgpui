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
  if (cache.lookup_count() != 2 || cache.entries().size() != 2) {
    return 10;
  }
  if (cache.lookups()[0].hit || cache.lookups()[1].hit) {
    return 11;
  }

  cgpui::vulkan_consume_text_draw(text, cache);
  if (cache.lookup_count() != 4 || cache.entries().size() != 2) {
    return 12;
  }

  return cache.lookups()[2].hit && cache.lookups()[3].hit &&
                 cache.entries()[0].atlas_bounds.origin.x == 4.0F &&
                 cache.entries()[1].atlas_bounds.origin.x == 14.0F
             ? 0
             : 13;
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

} // namespace

int main() {
  if (const int result = test_glyph_cache_records_lookup_miss_and_hit();
      result != 0) {
    return result;
  }
  if (const int result = test_text_draw_uses_glyph_cache_metadata();
      result != 0) {
    return result;
  }
  if (const int result =
          test_renderer_batches_by_clip_opacity_transform_and_kind();
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
