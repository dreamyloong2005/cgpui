// The Vulkan renderer on a composition surface (Win32SurfaceHandle::
// composition, as popup windows get) on each GPU vendor, chosen with
// VK_LOADER_VENDOR_ID_FILTER. A half-transparent red rect over a cleared
// (transparent) frame is captured as premultiplied 128, 0, 0, 128, and
// DirectComposition composites it over a green window: 128, 127, 0 on
// the screen, and the cleared half shows the green window, not black.
#include "cgpui/renderer/renderer.hpp"
#include "vulkan_vendor_filter_support.hpp"
#include "win32_composition_screen_support.hpp"

#include <chrono>
#include <iostream>

namespace {

int run_on_vendor(std::uint32_t vendor) {
  if (!cgpui_test::filter_vulkan_vendor(vendor)) return 1;
  cgpui_test::CompositionTestWindows windows(560, 200);
  if (!windows.valid()) return 3;
  const auto started = std::chrono::steady_clock::now();
  auto renderer = cgpui::create_vulkan_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = cgpui::Win32SurfaceHandle{.hinstance = GetModuleHandleW(nullptr),
                                                  .hwnd = windows.popup(),
                                                  .composition = true},
      .framebuffer_size = {64.0F, 64.0F},
      .scale = cgpui::DpiScale{1.0F},
      .transparent_background = true});
  if (!renderer) {
    std::cerr << "renderer: " << renderer.error().message << '\n';
    return 4;
  }
  std::cout << "renderer created in "
            << std::chrono::duration<double, std::milli>(
                   std::chrono::steady_clock::now() - started).count()
            << " ms\n";
  const cgpui::RendererSurfaceComposition composition = (*renderer)->surface_composition();
  if (!composition.alpha_composited() ||
      composition.composite_alpha != cgpui::RendererCompositeAlpha::pre_multiplied) {
    return 5;
  }
  for (int frame_index = 0; frame_index < 2; ++frame_index) {  // a reused frame too
    auto frame = (*renderer)->begin_frame();
    if (!frame || !(*frame)->request_pixel_capture()) return 6;
    (*frame)->clear({.r = 0.0F, .g = 0.0F, .b = 0.0F, .a = 0.0F});
    (*frame)->draw_rect(cgpui::SolidRect{
        .rect = {{0.0F, 0.0F}, {32.0F, 64.0F}}, .color = {.r = 1.0F, .a = 0.5F}});
    if (auto presented = (*frame)->present(); !presented) {
      std::cerr << "present: " << presented.error().message << '\n';
      return 7;
    }
  }
  const cgpui::RendererFramePixels* pixels = (*renderer)->last_frame_pixels();
  if (pixels == nullptr || pixels->width != 64 ||
      !cgpui_test::expect_texture_pixel(*pixels, 16, 32, {128, 0, 0, 128}) ||
      !cgpui_test::expect_texture_pixel(*pixels, 48, 32, {0, 0, 0, 0})) {
    return 8;
  }
  if (!windows.expect({0, 255, 0}, windows.backdrop_point()) ||
      !windows.expect({128, 127, 0}, windows.popup_point(16, 32)) ||
      !windows.expect({0, 255, 0}, windows.popup_point(48, 32))) {
    return 9;
  }
  return 0;
}

} // namespace

int main() {
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  const auto vendors = cgpui_test::vulkan_vendors();
  if (vendors.empty()) return 20;
  for (const std::uint32_t vendor : vendors) {
    if (const int result = run_on_vendor(vendor); result != 0) {
      std::cerr << "vendor 0x" << std::hex << vendor << " failed with " << std::dec << result
                << '\n';
      return result;
    }
  }
  return 0;
}
