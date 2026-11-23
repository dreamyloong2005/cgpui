#include "renderer_pixel_test_backend.hpp"
#include "cgpui/renderer/renderer_backend.hpp"

#include <cstdlib>
#include <iostream>

namespace cgpui_test {
cgpui::Result<std::unique_ptr<cgpui::Renderer>> create_pixel_test_renderer(
    const cgpui::RenderSurfaceDescriptor& descriptor) {
  // Own both environment snapshots before native creation can dispatch callbacks.
  const char* setting = std::getenv("CGPUI_RENDERER");
  const std::string requested = setting ? setting : "";
  const char* adapter_setting = std::getenv("CGPUI_DX12_ADAPTER");
  const std::string adapter = adapter_setting ? adapter_setting : "";
  const auto expected = parse_pixel_test_backend(requested);
  if (!expected) {
    report_pixel_test_error("selection", expected.error());
    return std::unexpected(expected.error());
  }
  auto renderer = *expected == cgpui::RendererBackend::dx12
      ? cgpui::create_dx12_renderer(descriptor)
      : cgpui::create_vulkan_renderer(descriptor);
  if (!renderer) {
    report_pixel_test_error("create requested=" + std::string(pixel_backend_name(*expected)), renderer.error());
    return std::unexpected(renderer.error());
  }
  if (!*renderer) {
    const cgpui::Error error{cgpui::ErrorCode::renderer_initialization_failed,
                            "pixel test factory returned a null renderer"};
    report_pixel_test_error("create", error);
    return std::unexpected(error);
  }
  const auto info = (*renderer)->backend_info();
  std::cout << "pixel renderer: requested=" << (requested.empty() ? "<unset>" : requested)
            << " expected=" << pixel_backend_name(*expected)
            << " actual=" << pixel_backend_name(info.backend)
            << " selection=" << static_cast<int>(info.selection)
            << " adapter=" << info.adapter_name << " vendor=" << info.vendor_id
            << " luid=" << (info.adapter_luid ? std::to_string(*info.adapter_luid) : "unavailable")
            << " software=" << info.software_adapter << " fallback=" << info.fallback_reason << '\n';
  const bool warp = *expected == cgpui::RendererBackend::dx12 && adapter == "warp";
  if (auto identity = validate_pixel_backend_info(*expected, info, warp); !identity) {
    report_pixel_test_error("identity", identity.error());
    return std::unexpected(identity.error());
  }
  return renderer;
}
} // namespace cgpui_test
