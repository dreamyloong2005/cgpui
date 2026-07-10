#include "cgpui/renderer/renderer.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <memory>
#include <string_view>
#include <utility>
#include <vector>

namespace {

class HiddenWindow {
 public:
  HiddenWindow() : instance_(GetModuleHandleW(nullptr)) {
    const wchar_t* class_name = L"CGPUIVulkanFrameLifetimeTestWindow";

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.lpfnWndProc = DefWindowProcW;
    window_class.hInstance = instance_;
    window_class.lpszClassName = class_name;
    RegisterClassExW(&window_class);

    hwnd_ = CreateWindowExW(
        0,
        class_name,
        L"CGPUI Vulkan Frame Lifetime Test",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        64,
        64,
        nullptr,
        nullptr,
        instance_,
        nullptr);
  }

  ~HiddenWindow() {
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

 private:
  HINSTANCE instance_ = nullptr;
  HWND hwnd_ = nullptr;
};

cgpui::TextDraw text_draw(std::string_view content) {
  const cgpui::FontDescriptor font{.family = "Inter"};
  return cgpui::TextDraw{
      .bounds =
          cgpui::Rect{
              .origin = cgpui::Point{.x = 4.0F, .y = 4.0F},
              .size = cgpui::Size{.width = 48.0F, .height = 24.0F},
          },
      .color = cgpui::Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
      .font = font,
      .content = std::string(content),
      .byte_length = content.size(),
      .font_size = 16.0F,
      .glyphs = cgpui::text_glyph_paint_metadata(
          cgpui::shape_text(content, font, 16.0F),
          cgpui::Point{.x = 4.0F, .y = 4.0F}),
  };
}

cgpui::TextDraw multi_page_text_draw() {
  cgpui::TextDraw draw{
      .bounds = cgpui::Rect{.size = cgpui::Size{64.0F, 64.0F}},
      .color = cgpui::Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
      .font = cgpui::FontDescriptor{.family = "Inter"},
      .content = "multi-page",
      .byte_length = 10,
      .font_size = 128.0F,
      .device_font_size = 128.0F,
  };
  draw.glyphs.reserve(9);
  for (std::size_t index = 0; index < 9; ++index) {
    draw.glyphs.push_back(cgpui::TextGlyphPaint{
        .key =
            cgpui::GlyphAtlasKey{
                .font_family = "Inter",
                .font_size = 128.0F,
                .scale = 1.0F,
                .device_font_size = 128.0F,
                .glyph_index = index,
                .glyph_id = static_cast<std::uint32_t>(index + 1),
                .byte_offset = index,
                .byte_length = 1,
            },
        .origin = cgpui::Point{.x = static_cast<float>(index) * 128.0F},
        .advance = 128.0F,
        .device_origin =
            cgpui::Point{.x = static_cast<float>(index) * 128.0F},
        .device_advance = 128.0F,
    });
  }
  return draw;
}

bool present_draw_frame(cgpui::Renderer& renderer, cgpui::TextDraw draw) {
  auto frame = renderer.begin_frame();
  if (!frame) {
    return false;
  }
  (*frame)->clear(
      cgpui::Color{.r = 0.30F, .g = 0.33F, .b = 0.35F, .a = 1.0F});
  (*frame)->draw_text(std::move(draw));
  return static_cast<bool>((*frame)->present());
}

bool present_text_frame(cgpui::Renderer& renderer, std::string_view content) {
  return present_draw_frame(renderer, text_draw(content));
}

int test_frame_outlives_renderer(const HiddenWindow& window) {
  std::unique_ptr<cgpui::RenderFrame> frame;
  {
    auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
        .native_surface = window.surface(),
        .framebuffer_size = cgpui::Size{64.0F, 64.0F},
        .scale = cgpui::DpiScale{1.0F}});
    if (!renderer) {
      return 1;
    }

    auto frame_result = (*renderer)->begin_frame();
    if (!frame_result) {
      return 2;
    }
    frame = std::move(*frame_result);
  }

  frame->clear(cgpui::Color{.r = 0.30F, .g = 0.33F, .b = 0.35F, .a = 1.0F});
  frame->draw_text(text_draw("atlas"));
  return frame->present() ? 0 : 3;
}

int test_incremental_glyph_atlas_frames(const HiddenWindow& window) {
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.surface(),
      .framebuffer_size = cgpui::Size{64.0F, 64.0F},
      .scale = cgpui::DpiScale{1.0F}});
  if (!renderer) {
    return 5;
  }
  if (!present_text_frame(**renderer, "ab")) {
    return 6;
  }
  if (!present_text_frame(**renderer, "ab")) {
    return 7;
  }
  return present_text_frame(**renderer, "abc") ? 0 : 8;
}

int test_live_frame_diagnostic_snapshot(const HiddenWindow& window) {
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.surface(),
      .framebuffer_size = cgpui::Size{64.0F, 64.0F},
      .scale = cgpui::DpiScale{1.0F}});
  if (!renderer) {
    return 28;
  }
  auto frame = (*renderer)->begin_frame();
  if (!frame) {
    return 29;
  }
  (*frame)->draw_rect(cgpui::SolidRect{
      .rect = {.size = {.width = 8.0F, .height = 8.0F}},
      .color = {.r = 1.0F, .g = 0.0F, .b = 0.0F, .a = 1.0F},
  });
  (*frame)->draw_text_selection(cgpui::TextSelectionDraw{});
  (*frame)->draw_text_caret(cgpui::TextCaretDraw{});
  if (!(*frame)->present()) {
    return 30;
  }
  const cgpui::RendererFrameDiagnosticSnapshot* snapshot =
      (*renderer)->last_frame_diagnostic_snapshot();
  return snapshot != nullptr &&
                 snapshot->work.planned_work.command_count == 3 &&
                 snapshot->work.submitted_work.command_count == 1 &&
                 snapshot->planned_draws.total_count == 3 &&
                 snapshot->submitted_draws.total_count == 1 &&
                 snapshot->dropped_resources.resources.size() == 2 &&
                 snapshot->timings.total_nanoseconds != 0
             ? 0
             : 31;
}

int test_image_texture_resource_frame(const HiddenWindow& window) {
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.surface(),
      .framebuffer_size = cgpui::Size{64.0F, 64.0F},
      .scale = cgpui::DpiScale{1.0F}});
  if (!renderer) {
    return 11;
  }
  auto frame = (*renderer)->begin_frame();
  if (!frame) {
    return 12;
  }
  (*frame)->clear(
      cgpui::Color{.r = 0.30F, .g = 0.33F, .b = 0.35F, .a = 1.0F});
  (*frame)->draw_image(cgpui::ImageDraw{
      .bounds = {.size = {.width = 16.0F, .height = 16.0F}},
      .asset =
          cgpui::ImageAssetDescriptor{
              .id = cgpui::ImageAssetId{41},
              .logical_size = {.width = 2.0F, .height = 2.0F},
              .pixel_width = 2,
              .pixel_height = 2,
              .stride = 8,
              .format = cgpui::ImageFormat::rgba8_unorm,
              .byte_size = 16,
          },
  });
  return (*frame)->present() ? 0 : 13;
}

int test_image_texture_cache_idle_frame(const HiddenWindow& window) {
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.surface(),
      .framebuffer_size = cgpui::Size{64.0F, 64.0F},
      .scale = cgpui::DpiScale{1.0F}});
  if (!renderer) {
    return 14;
  }
  cgpui::ImageAsset image{
      .id = cgpui::ImageAssetId{42},
      .logical_size = {.width = 2.0F, .height = 2.0F},
      .bitmap =
          cgpui::DecodedImageBitmap{
              .width = 2,
              .height = 2,
              .stride = 8,
              .format = cgpui::ImageFormat::rgba8_unorm,
              .pixels =
                  std::vector<std::uint8_t>{
                      255, 0, 0, 255, 0, 255, 0, 255,
                      0, 0, 255, 255, 255, 255, 255, 255,
                  },
          },
  };
  const cgpui::ImageAssetDescriptor descriptor =
      cgpui::describe_image_asset(image);
  {
    auto frame = (*renderer)->begin_frame();
    if (!frame) {
      return 15;
    }
    (*frame)->upload_image(image);
    (*frame)->draw_image(cgpui::ImageDraw{
        .bounds = {.size = {.width = 16.0F, .height = 16.0F}},
        .asset = descriptor,
    });
    if (!(*frame)->present()) {
      return 16;
    }
  }
  {
    auto frame = (*renderer)->begin_frame();
    if (!frame) {
      return 17;
    }
    (*frame)->clear(
        cgpui::Color{.r = 0.30F, .g = 0.33F, .b = 0.35F, .a = 1.0F});
    if (!(*frame)->present()) {
      return 18;
    }
  }
  auto frame = (*renderer)->begin_frame();
  if (!frame) {
    return 19;
  }
  (*frame)->draw_image(cgpui::ImageDraw{
      .bounds = {.size = {.width = 16.0F, .height = 16.0F}},
      .asset = descriptor,
  });
  return (*frame)->present() ? 0 : 20;
}

int test_image_texture_upload_frame(const HiddenWindow& window) {
  return test_image_texture_cache_idle_frame(window);
}

int test_image_texture_invalidation_frame(const HiddenWindow& window) {
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.surface(),
      .framebuffer_size = cgpui::Size{64.0F, 64.0F},
      .scale = cgpui::DpiScale{1.0F}});
  if (!renderer) {
    return 21;
  }
  cgpui::ImageAsset image{
      .id = cgpui::ImageAssetId{43},
      .logical_size = {.width = 2.0F, .height = 2.0F},
      .bitmap =
          cgpui::DecodedImageBitmap{
              .width = 2,
              .height = 2,
              .stride = 8,
              .format = cgpui::ImageFormat::rgba8_unorm,
              .pixels = std::vector<std::uint8_t>(16, 255),
          },
  };
  const cgpui::ImageAssetDescriptor descriptor =
      cgpui::describe_image_asset(image);
  {
    auto frame = (*renderer)->begin_frame();
    if (!frame) {
      return 22;
    }
    (*frame)->upload_image(image);
    (*frame)->draw_image(cgpui::ImageDraw{
        .bounds = {.size = {.width = 16.0F, .height = 16.0F}},
        .asset = descriptor,
    });
    if (!(*frame)->present()) {
      return 23;
    }
  }
  {
    auto frame = (*renderer)->begin_frame();
    if (!frame) {
      return 24;
    }
    (*frame)->invalidate_image(image.id);
    (*frame)->draw_image(cgpui::ImageDraw{
        .bounds = {.size = {.width = 16.0F, .height = 16.0F}},
        .asset = descriptor,
    });
    if (!(*frame)->present()) {
      return 25;
    }
  }
  auto frame = (*renderer)->begin_frame();
  if (!frame) {
    return 26;
  }
  (*frame)->invalidate_image(image.id);
  (*frame)->upload_image(image);
  (*frame)->draw_image(cgpui::ImageDraw{
      .bounds = {.size = {.width = 16.0F, .height = 16.0F}},
      .asset = descriptor,
  });
  return (*frame)->present() ? 0 : 27;
}

int test_multi_page_glyph_atlas_frame(const HiddenWindow& window) {
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.surface(),
      .framebuffer_size = cgpui::Size{64.0F, 64.0F},
      .scale = cgpui::DpiScale{1.0F}});
  if (!renderer) {
    return 9;
  }
  return present_draw_frame(**renderer, multi_page_text_draw()) ? 0 : 10;
}

} // namespace

int main() {
  HiddenWindow window;
  if (!window.valid()) {
    return 4;
  }
  if (const int result = test_frame_outlives_renderer(window); result != 0) {
    return result;
  }
  if (const int result = test_incremental_glyph_atlas_frames(window);
      result != 0) {
    return result;
  }
  if (const int result = test_live_frame_diagnostic_snapshot(window);
      result != 0) {
    return result;
  }
  if (const int result = test_image_texture_resource_frame(window);
      result != 0) {
    return result;
  }
  if (const int result = test_image_texture_upload_frame(window); result != 0) {
    return result;
  }
  if (const int result = test_image_texture_invalidation_frame(window);
      result != 0) {
    return result;
  }
  return test_multi_page_glyph_atlas_frame(window);
}
