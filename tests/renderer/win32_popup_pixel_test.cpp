// Win32 popup interop, once per GPU vendor. This verifies the private
// presentation path used by Vulkan application popups without exposing
// platform-specific presentation modes to application code.
// (VK_LOADER_VENDOR_ID_FILTER): a parent window drawn green by its own
// renderer (VkSwapchainKHR), and a popup and a pointer-transparent
// (layered) popup, each with a renderer on its composition surface as the
// runtime creates it. Both draw a half-transparent red rect over a
// transparent clear: the texture holds premultiplied 128, 0, 0, 128, the
// screen shows 128, 127, 0 over the parent's green, and the cleared half
// shows the green parent instead of a black frame.
#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"
#include "vulkan_vendor_filter_support.hpp"
#include "win32_composition_screen_support.hpp"

#include <chrono>
#include <iostream>
#include <memory>

namespace {

HWND hwnd_of(const cgpui::PlatformWindow& window) {
  return static_cast<HWND>(std::get<cgpui::Win32SurfaceHandle>(window.native_surface()).hwnd);
}

std::unique_ptr<cgpui::Renderer> renderer_for(const cgpui::PlatformWindow& window,
                                              bool transparent) {
  const cgpui::WindowState state = window.state();
  auto renderer = cgpui::create_vulkan_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.native_surface(),
      .framebuffer_size = state.framebuffer_size,
      .scale = state.scale,
      .transparent_background = transparent});
  if (!renderer) {
    std::cerr << "renderer: " << renderer.error().message << '\n';
    return nullptr;
  }
  return std::move(*renderer);
}

// Draws the half-transparent rect into `popup` and checks texture and screen.
int check_popup(cgpui::PlatformWindow& popup) {
  const auto started = std::chrono::steady_clock::now();
  auto renderer = renderer_for(popup, true);
  if (renderer == nullptr) return 1;
  if (!renderer->popup_windows_supported()) return 7;
  std::cout << "popup renderer created in "
            << std::chrono::duration<double, std::milli>(
                   std::chrono::steady_clock::now() - started).count()
            << " ms\n";
  if (!renderer->surface_composition().alpha_composited()) return 2;
  for (int frame_index = 0; frame_index < 2; ++frame_index) {
    auto frame = renderer->begin_frame();
    if (!frame || !(*frame)->request_pixel_capture()) return 3;
    (*frame)->clear({.r = 0.0F, .g = 0.0F, .b = 0.0F, .a = 0.0F});
    (*frame)->draw_rect(cgpui::SolidRect{
        .rect = {{0.0F, 0.0F}, {32.0F, 48.0F}}, .color = {.r = 1.0F, .a = 0.5F}});
    if (auto presented = (*frame)->present(); !presented) {
      std::cerr << "present: " << presented.error().message << '\n';
      return 4;
    }
  }
  const cgpui::RendererFramePixels* pixels = renderer->last_frame_pixels();
  if (pixels == nullptr) return 5;
  const std::uint32_t middle_row = pixels->height / 2;
  if (!cgpui_test::expect_texture_pixel(*pixels, pixels->width / 4, middle_row,
                                        {128, 0, 0, 128}) ||
      !cgpui_test::expect_texture_pixel(*pixels, pixels->width * 3 / 4, middle_row,
                                        {0, 0, 0, 0})) {
    return 5;
  }
  RECT rect{};
  GetWindowRect(hwnd_of(popup), &rect);
  const LONG middle = (rect.top + rect.bottom) / 2;
  const LONG quarter = (rect.right - rect.left) / 4;
  return cgpui_test::expect_screen_color({128, 127, 0}, {rect.left + quarter, middle}) &&
                 cgpui_test::expect_screen_color({0, 255, 0}, {rect.right - quarter, middle})
             ? 0
             : 6;
}

int run_on_vendor(cgpui::PlatformApplication& application, std::uint32_t vendor) {
  if (!cgpui_test::filter_vulkan_vendor(vendor)) return 10;
  const auto ignore = [](const cgpui::PlatformEvent&) {};
  auto parent = application.create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Win32 Popup Pixel Test", .size = {240.0F, 120.0F}},
      ignore);
  if (!parent) return 11;
  const POINT origin = cgpui_test::work_area_point(1400, 200);
  SetWindowPos(hwnd_of(**parent), HWND_TOPMOST, origin.x, origin.y, 0, 0,
               SWP_NOSIZE | SWP_NOACTIVATE);
  auto parent_renderer = renderer_for(**parent, false);
  if (parent_renderer == nullptr) return 12;
  auto parent_frame = parent_renderer->begin_frame();
  if (!parent_frame) return 13;
  (*parent_frame)->clear({.r = 0.0F, .g = 1.0F, .b = 0.0F, .a = 1.0F});
  if (!(*parent_frame)->present()) return 14;
  cgpui_test::pump_messages_for(100);

  // A popup (menus) and a pointer-transparent one (tooltips), side by side.
  for (const bool pointer_transparent : {false, true}) {
    cgpui::WindowDescriptor descriptor{
        .size = {64.0F, 48.0F},
        .kind = cgpui::WindowKind::popup,
        .popup = cgpui::WindowPopupPlacement{
            .anchor = {{pointer_transparent ? 112.0F : 16.0F, 16.0F}, {0.0F, 0.0F}}}};
    descriptor.popup_options.pointer_transparent = pointer_transparent;
    auto popup = application.create_child_window(descriptor, **parent, ignore);
    if (!popup) return 15;
    cgpui_test::pump_messages_for(100);
    std::cout << (pointer_transparent ? "pointer-transparent popup\n" : "popup\n");
    if (const int result = check_popup(**popup); result != 0) return 20 + result;
  }
  return 0;
}

} // namespace

int main() {
  auto application = cgpui::create_platform_application();
  if (!application || !(*application)->popup_windows_supported()) return 1;
  const auto vendors = cgpui_test::vulkan_vendors();
  if (vendors.empty()) return 2;
  for (const std::uint32_t vendor : vendors) {
    if (const int result = run_on_vendor(**application, vendor); result != 0) {
      std::cerr << "vendor 0x" << std::hex << vendor << " failed with " << std::dec << result
                << '\n';
      return result;
    }
  }
  return 0;
}
