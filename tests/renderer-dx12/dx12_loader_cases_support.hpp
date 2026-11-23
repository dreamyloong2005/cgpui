#pragma once

#include "dx12_test_support.hpp"
#include <cstdlib>
#include <string_view>

namespace cgpui_test {
inline bool loader_case_pixels(cgpui::Renderer& renderer) {
  auto frame = renderer.begin_frame();
  if (!frame) return false;
  (*frame)->clear({0, 0, 1, 1});
  (*frame)->draw_rect({.rect = {{8, 8}, {16, 16}}, .color = {1, 0, 0, 1}});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present()) return false;
  const auto* pixels = renderer.last_frame_pixels();
  return pixels && pixels->width == 64 && pixels->height == 64 &&
      dx12_pixel(pixels, 12, 12, {255, 0, 0, 255}) &&
      dx12_pixel(pixels, 40, 40, {0, 0, 255, 255});
}

inline int loader_child_case(std::string_view mode) {
  using namespace cgpui;
  if (GetModuleHandleW(L"vulkan-1.dll")) return dx12_fail(20, "child imported Vulkan before main");
  const bool dx12 = mode == "dx12" || mode == "dx12-invalid";
  const bool healthy = mode == "healthy-vulkan";
  const char* backend = dx12 ? "dx12" : mode == "default" ? "" : mode == "auto" ? "auto" : "vulkan";
  if (mode != "dx12" && mode != "dx12-invalid" && mode != "vulkan" && mode != "default" &&
      mode != "auto" && !healthy) return dx12_fail(21, "unknown loader child mode");
  if (_putenv_s("CGPUI_RENDERER", backend) ||
      _putenv_s("CGPUI_DX12_ADAPTER", mode == "dx12-invalid" ? "invalid-adapter" : "warp"))
    return dx12_fail(22, "cannot select loader child environment");
  {
    Dx12Window window;
    if (!window.hwnd) return dx12_fail(23, "loader child native window unavailable");
    auto renderer = create_renderer(window.descriptor());
    if (mode == "dx12" || healthy) {
      if (!renderer) return dx12_fail(24, renderer.error().message.c_str());
      const auto info = (*renderer)->backend_info();
      const auto expected = healthy ? RendererBackend::vulkan : RendererBackend::dx12;
      const auto selection = healthy ? RendererBackendSelection::vulkan : RendererBackendSelection::dx12;
      if (info.backend != expected || info.selection != selection || info.adapter_name.empty() ||
          !info.device_generation || !info.fallback_reason.empty() ||
          (!healthy && (!info.software_adapter || !info.adapter_luid)) ||
          (healthy != (GetModuleHandleW(L"vulkan-1.dll") != nullptr)) || !loader_case_pixels(**renderer))
        return dx12_fail(25, "loader child backend metadata/module state or real captured pixels differ");
      std::cout << "LOADER_CASE mode=" << mode << " pixels=64x64 adapter=" << info.adapter_name << '\n';
    } else {
      const auto expected = dx12 ? ErrorCode::invalid_argument : ErrorCode::renderer_initialization_failed;
      const char* diagnostic = dx12 ? "CGPUI_DX12_ADAPTER" : "vulkan-1.dll";
      if (renderer || renderer.error().code != expected || renderer.error().message.find(diagnostic) == std::string::npos)
        return dx12_fail(26, "explicit backend failure was masked, fell back, or lost its loader diagnostic");
      std::cout << "LOADER_CASE mode=" << mode << " refused=" << renderer.error().message << '\n';
    }
    if (!healthy && GetModuleHandleW(L"vulkan-1.dll"))
      return dx12_fail(27, "DX12 or failed Vulkan initialization left a Vulkan loader resident");
  }
  if (!healthy && GetModuleHandleW(L"vulkan-1.dll"))
    return dx12_fail(28, "isolated child teardown loaded Vulkan");
  if (healthy && !GetModuleHandleW(L"vulkan-1.dll"))
    return dx12_fail(29, "successful Vulkan loader was unloaded during renderer teardown");
  return 0;
}
} // namespace cgpui_test
