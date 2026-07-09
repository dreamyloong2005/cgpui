#include "cgpui/renderer/renderer.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <memory>

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

} // namespace

int main() {
  HiddenWindow window;
  if (!window.valid()) {
    return 4;
  }

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
  frame->draw_text(cgpui::TextDraw{
      .bounds =
          cgpui::Rect{
              .origin = cgpui::Point{.x = 4.0F, .y = 4.0F},
              .size = cgpui::Size{.width = 48.0F, .height = 24.0F},
          },
      .color = cgpui::Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
      .font = cgpui::FontDescriptor{.family = "Inter"},
      .content = "atlas",
      .byte_length = 5,
      .font_size = 16.0F,
      .glyphs = cgpui::text_glyph_paint_metadata(
          cgpui::shape_text(
              "atlas",
              cgpui::FontDescriptor{.family = "Inter"},
              16.0F),
          cgpui::Point{.x = 4.0F, .y = 4.0F}),
  });
  const auto presented = frame->present();
  return presented ? 0 : 3;
}
