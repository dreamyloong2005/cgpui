// The Vulkan renderer on a composition surface (Win32SurfaceHandle::
// composition, as popup windows get) on each GPU vendor, chosen with
// VK_LOADER_VENDOR_ID_FILTER. A half-transparent red rect over a cleared
// (transparent) frame is captured as premultiplied 128, 0, 0, 128, and
// DirectComposition composites it over a green window: 128, 127, 0 on
// the screen, and the cleared half shows the green window, not black.
#include "cgpui/renderer/renderer.hpp"
#include "win32_composition_screen_support.hpp"

#include <vulkan/vulkan.h>

#include <chrono>
#include <cstdio>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace {

using cgpui_test::ScreenColor;

// The Vulkan 1.1 devices the loader shows under the current environment.
std::vector<VkPhysicalDeviceProperties> visible_devices() {
  const VkApplicationInfo application{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                      .apiVersion = VK_API_VERSION_1_1};
  const VkInstanceCreateInfo create{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                    .pApplicationInfo = &application};
  VkInstance instance = VK_NULL_HANDLE;
  if (vkCreateInstance(&create, nullptr, &instance) != VK_SUCCESS) return {};
  std::uint32_t count = 0;
  vkEnumeratePhysicalDevices(instance, &count, nullptr);
  std::vector<VkPhysicalDevice> devices(count);
  vkEnumeratePhysicalDevices(instance, &count, devices.data());
  std::vector<VkPhysicalDeviceProperties> result;
  for (VkPhysicalDevice device : devices) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device, &properties);
    if (properties.apiVersion >= VK_API_VERSION_1_1) result.push_back(properties);
  }
  vkDestroyInstance(instance, nullptr);
  return result;
}

bool pixel_is(const cgpui::RendererFramePixels& pixels, std::uint32_t x, std::uint32_t y,
              std::array<int, 4> expected) {
  const auto pixel = pixels.pixel_rgba8(x, y);
  if (!pixel) return false;
  std::cout << "texture " << x << "," << y << ": " << int((*pixel)[0]) << ' '
            << int((*pixel)[1]) << ' ' << int((*pixel)[2]) << ' ' << int((*pixel)[3]) << '\n';
  for (std::size_t channel = 0; channel < 4; ++channel) {
    if (std::abs(int((*pixel)[channel]) - expected[channel]) > 2) return false;
  }
  return true;
}

int run_on_vendor(std::uint32_t vendor) {
  char filter[16]{};
  std::snprintf(filter, sizeof(filter), "0x%04x", vendor);
  SetEnvironmentVariableA("VK_LOADER_VENDOR_ID_FILTER", filter);
  const auto devices = visible_devices();
  for (const auto& device : devices) {
    std::cout << "VK_LOADER_VENDOR_ID_FILTER=" << filter << ": " << device.deviceName << '\n';
    if (device.vendorID != vendor) return 1;  // the filter did not apply
  }
  if (devices.empty()) return 2;
  cgpui_test::CompositionTestWindows windows(560, 200);
  if (!windows.valid()) return 3;
  const auto started = std::chrono::steady_clock::now();
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
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
  if (pixels == nullptr || pixels->width != 64 || !pixel_is(*pixels, 16, 32, {128, 0, 0, 128}) ||
      !pixel_is(*pixels, 48, 32, {0, 0, 0, 0})) {
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
  std::set<std::uint32_t> vendors;
  for (const auto& device : visible_devices()) {
    std::cout << "unfiltered: " << device.deviceName << '\n';
    vendors.insert(device.vendorID);
  }
  if (vendors.empty()) return 20;
  for (std::uint32_t vendor : vendors) {
    if (const int result = run_on_vendor(vendor); result != 0) {
      std::cerr << "vendor 0x" << std::hex << vendor << " failed with " << std::dec << result
                << '\n';
      return result;
    }
  }
  return 0;
}
