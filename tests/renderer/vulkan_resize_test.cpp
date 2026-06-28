#include "cgpui/renderer/renderer.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {

class HiddenWindow {
 public:
  HiddenWindow() : instance_(GetModuleHandleW(nullptr)) {
    const wchar_t* class_name = L"CGPUIVulkanResizeTestWindow";

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.lpfnWndProc = DefWindowProcW;
    window_class.hInstance = instance_;
    window_class.lpszClassName = class_name;
    RegisterClassExW(&window_class);

    hwnd_ = CreateWindowExW(
        0,
        class_name,
        L"CGPUI Vulkan Resize Test",
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

  [[nodiscard]] bool valid() const { return instance_ != nullptr && hwnd_ != nullptr; }

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

  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.surface(),
      .framebuffer_size = cgpui::Size{64.0F, 64.0F},
      .scale = cgpui::DpiScale{1.0F}});
  if (!renderer) {
    return 1;
  }

  const auto resized =
      (*renderer)->resize(cgpui::Size{0.0F, 64.0F}, cgpui::DpiScale{1.0F});
  if (resized) {
    return 2;
  }

  return resized.error().code == cgpui::ErrorCode::renderer_initialization_failed
      ? 0
      : 3;
}
