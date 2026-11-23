#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <algorithm>

namespace {
std::string read(const char* path) {
  std::ifstream stream(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  return {std::istreambuf_iterator<char>(stream), {}};
}
bool has(const std::string& text, const char* token) {
  return text.find(token) != std::string::npos;
}
bool bounded(const std::string& text, std::size_t lines) {
  return !text.empty() && std::count(text.begin(), text.end(), '\n') <= lines;
}
}

int main() {
  const auto application = read("src/platform/win32/win32_application.cpp");
  const auto registry = read("src/platform/win32/win32_window_registry.cpp");
  if (!bounded(registry, 60) || !has(application, "windows_.visit(") ||
      has(application, "std::vector<Win32Window*> windows_") ||
      !bounded(read("src/platform/win32/win32_window_registry_internal.hpp"), 45)) return 1;
  const auto dispatch = read("src/platform/win32/win32_window_dispatch_lifetime.cpp");
  if (!bounded(dispatch, 30) || !has(dispatch, "registration_->window = nullptr") ||
      !bounded(read("src/platform/win32/win32_window_dispatch_lifetime_internal.hpp"), 40) ||
      !has(read("src/platform/win32/win32_window.cpp"), "invalidate_window_registration()")) return 7;
  const auto retry = read("src/ui/runtime_frame_retry.cpp");
  const auto readiness = read("src/ui/runtime_frame_readiness.cpp");
  if (!bounded(retry, 30) || !has(retry, "ErrorCode::frame_retry_required") ||
      has(retry, "error.message") || has(retry, "Vulkan") || has(retry, "frame_not_ready") ||
      !bounded(readiness, 280) || !has(readiness, "return runtime_retry_frame(runtime, window, error)") ||
      !has(read("src/ui/runtime_rendering.cpp"), "frame_readiness_.handle(") ||
      !has(read("src/ui/runtime_window_rendering.cpp"), "frame_readiness_.handle(")) return 2;
  const auto presentation = read("src/renderer/vulkan/vulkan_presentation.cpp");
  const auto first_retry = presentation.find("ErrorCode::frame_retry_required");
  if (first_retry == std::string::npos ||
      presentation.find("ErrorCode::frame_retry_required", first_retry + 1) == std::string::npos) return 3;
  const auto surface = read("src/renderer/metal/metal_surface.mm");
  if (!has(surface, "state.layer.framebufferOnly = NO") ||
      has(surface, "state.layer.framebufferOnly = YES")) return 4;
  const auto system = read("src/platform/win32/win32_system_application.cpp");
  const auto host = read("src/platform/win32/win32_composition_host.cpp");
  const auto platform = read("packages/platform-win32/mcpp.toml");
  if (has(system, "renderer/") ||
      !has(system, "win32_composition_host_available()") ||
      !bounded(host, 50) || !has(host, "DCompositionCreateDevice(") ||
      has(host, "D3D11") || has(platform, "renderer-win32-composition") ||
      !has(platform, "dcomp.lib")) return 5;
  const auto retirement = read("src/renderer/vulkan/vulkan_image_upload_retirement.cpp");
  const auto frame = read("src/renderer/vulkan/vulkan_image_texture_frame.cpp");
  const auto cache = read("src/renderer/vulkan/vulkan_image_texture_cache.cpp");
  if (!bounded(retirement, 65) || !has(retirement, "resources.submitted") ||
      !has(retirement, "upload.descriptor.pixel_width") ||
      !has(frame, "retained_uploads = vulkan_prepare_image_upload_retirement(") ||
      !has(frame, "image_texture_resources_, retained_uploads") ||
      !has(cache, "for (ImageAssetId asset_id : retained_uploads)") ||
      !has(read("src/renderer/vulkan/vulkan_image_texture_upload_recording.cpp"),
           "upload_resources.submitted = true")) return 6;
  return 0;
}
