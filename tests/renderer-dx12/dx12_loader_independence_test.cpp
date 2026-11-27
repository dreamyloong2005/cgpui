#include "../support/frame_wait_test_support.hpp"
#include "../support/win32_desktop_test_support.hpp"
#include "dx12_test_support.hpp"

#include <cstdlib>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  // This must precede environment selection, window creation and every renderer
  // call: an ordinary PE import loads Vulkan before main and must fail here.
  if (GetModuleHandleW(L"vulkan-1.dll") != nullptr)
    return dx12_fail(1, "Vulkan loader was imported before DX12 selection");
  if (_putenv_s("CGPUI_RENDERER", "dx12") || _putenv_s("CGPUI_DX12_ADAPTER", "warp"))
    return dx12_fail(2, "cannot select explicit DX12/WARP");
  Win32DesktopTestLease desktop;
  if (!desktop.ready()) return dx12_fail(13, "exclusive desktop fixture unavailable");
  {
    Dx12Window window;
    if (!window.hwnd) return dx12_fail(3, "native opaque window unavailable");
    ShowWindow(window.hwnd, SW_SHOWNOACTIVATE);
    UpdateWindow(window.hwnd);
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
    if (!IsWindowVisible(window.hwnd) || IsIconic(window.hwnd))
      return dx12_fail(4, "native opaque window was not shown");

    // Exercise the public dispatcher, not the backend-specific factory.
    auto renderer = create_renderer(window.descriptor());
    if (!renderer) return dx12_fail(5, renderer.error().message.c_str());
    const auto info = (*renderer)->backend_info();
    const auto composition = (*renderer)->surface_composition();
    if (info.backend != RendererBackend::dx12 || info.selection != RendererBackendSelection::dx12 ||
        !info.software_adapter || info.adapter_name.empty() || !info.adapter_luid || !info.device_generation ||
        !composition.supported || composition.transparent_requested ||
        composition.composite_alpha != RendererCompositeAlpha::opaque)
      return dx12_fail(6, "dispatcher did not report real opaque DX12/WARP selection");
    auto frame = cgpui_test::wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(7, frame.error().message.c_str());
    (*frame)->clear({0, 0, 1, 1});
    (*frame)->draw_rect({.rect = {{8, 8}, {16, 16}}, .color = {1, 0, 0, 1}});
    if (!(*frame)->request_pixel_capture()) return dx12_fail(8, "DX12 capture request failed");
    const auto presented = (*frame)->present();
    if (!presented) return dx12_fail(9, presented.error().message.c_str());
    const auto* pixels = (*renderer)->last_frame_pixels();
    if (!pixels || pixels->width != 64 || pixels->height != 64 ||
        !dx12_pixel(pixels, 12, 12, {255, 0, 0, 255}) ||
        !dx12_pixel(pixels, 40, 40, {0, 0, 255, 255}))
      return dx12_fail(10, "DX12 public dispatcher clear/rectangle capture differs");
    if (GetModuleHandleW(L"vulkan-1.dll") != nullptr)
      return dx12_fail(11, "DX12 creation or presentation loaded Vulkan");
    std::cout << "DX12_LOADER_INDEPENDENCE adapter=" << info.adapter_name
              << " software=" << info.software_adapter << " generation=" << info.device_generation
              << " capture=64x64 Vulkan=absent\n";
  }
  if (GetModuleHandleW(L"vulkan-1.dll") != nullptr)
    return dx12_fail(12, "DX12 teardown loaded Vulkan");
  return 0;
}
