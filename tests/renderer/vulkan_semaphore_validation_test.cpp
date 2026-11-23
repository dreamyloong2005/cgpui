// Several plain frames through a real swapchain with the Khronos validation
// layer enabled; any validation error fails the test. Guards the per-image
// render-finished semaphores (VUID-vkQueueSubmit-pSignalSemaphores-00067).
#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"

#include <vulkan/vulkan.h>
#if defined(_WIN32)
#include "vulkan_loader_windows_internal.hpp"
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace {

constexpr int kFrames = 8;

void set_env(const char* name, const std::string& value) {
#if defined(_WIN32)
  (void)_putenv_s(name, value.c_str());
#else
  (void)setenv(name, value.c_str(), 1);
#endif
}

bool validation_layer_available() {
#if defined(_WIN32)
  const auto enumerate_layers = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(
      cgpui::vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceLayerProperties"));
  if (!enumerate_layers) return false;
#else
  const auto enumerate_layers = &vkEnumerateInstanceLayerProperties;
#endif
  std::uint32_t count = 0;
  if (enumerate_layers(&count, nullptr) != VK_SUCCESS) {
    return false;
  }
  std::vector<VkLayerProperties> layers(count);
  if (enumerate_layers(&count, layers.data()) != VK_SUCCESS) {
    return false;
  }
  for (const VkLayerProperties& layer : layers) {
    if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
      return true;
    }
  }
  return false;
}

// The layer writes every error to `log` (layer settings through the
// environment: VK_LAYER_<SETTING> and VK_KHRONOS_VALIDATION_<SETTING>).
void enable_validation(const std::filesystem::path& log) {
  set_env("VK_INSTANCE_LAYERS", "VK_LAYER_KHRONOS_validation");
  for (const char* prefix : {"VK_LAYER_", "VK_KHRONOS_VALIDATION_"}) {
    set_env((std::string(prefix) + "DEBUG_ACTION").c_str(),
            "VK_DBG_LAYER_ACTION_LOG_MSG");
    set_env((std::string(prefix) + "REPORT_FLAGS").c_str(), "error");
    set_env((std::string(prefix) + "LOG_FILENAME").c_str(), log.string());
  }
}

std::string read_file(const std::filesystem::path& path) {
  std::ifstream stream(path);
  return stream ? std::string{std::istreambuf_iterator<char>(stream), {}}
                : std::string{};
}

} // namespace

int main() {
#if defined(_WIN32)
  if (const auto loader = cgpui::ensure_vulkan_windows_loader(); !loader) {
    std::cerr << loader.error().message << '\n';
    return 1;
  }
#else
  const char* wayland_display = std::getenv("WAYLAND_DISPLAY");
  if (wayland_display == nullptr || std::string_view(wayland_display).empty()) {
    std::cout << "WAYLAND_DISPLAY is not set; skipping the validation run\n";
    return 0;
  }
#endif
  if (!validation_layer_available()) {
    std::cout << "VK_LAYER_KHRONOS_validation is not installed; skipping\n";
    return 0;
  }
  const std::filesystem::path log =
      std::filesystem::temp_directory_path() / "cgpui_vulkan_semaphore_validation.log";
  std::error_code ignored;
  std::filesystem::remove(log, ignored);
  enable_validation(log);

  auto app = cgpui::create_platform_application();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 2;
  }
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Vulkan Validation Test",
                              .size = cgpui::Size{160.0F, 120.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          (*app)->quit();
        }
      });
  if (!window) {
    std::cerr << window.error().message << '\n';
    return 3;
  }
  const cgpui::WindowState state = (*window)->state();
  auto renderer = cgpui::create_vulkan_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = (*window)->native_surface(),
      .framebuffer_size = state.framebuffer_size,
      .scale = state.scale});
  if (!renderer) {
    std::cerr << renderer.error().message << '\n';
    return 4;
  }
  for (int index = 0; index < kFrames; ++index) {
    // Halfway, recreate the swapchain (and its semaphores).
    if (index == kFrames / 2) {
      if (auto resized = (*renderer)->resize(state.framebuffer_size, state.scale);
          !resized) {
        std::cerr << resized.error().message << '\n';
        return 7;
      }
    }
    auto frame = (*renderer)->begin_frame();
    if (!frame) {
      std::cerr << frame.error().message << '\n';
      return 5;
    }
    (*frame)->clear(cgpui::Color{.r = 0.1F * static_cast<float>(index % 4),
                                 .g = 0.2F, .b = 0.3F, .a = 1.0F});
    if (auto presented = (*frame)->present(); !presented) {
      std::cerr << presented.error().message << '\n';
      return 6;
    }
  }
  renderer->reset();
  (*window)->request_close();
  (void)(*app)->run();

  const std::string messages = read_file(log);
  if (messages.find("Validation Error") != std::string::npos ||
      messages.find("VUID-") != std::string::npos) {
    std::cerr << messages << '\n';
    return 20;
  }
  return 0;
}
