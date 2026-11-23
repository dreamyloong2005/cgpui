#include "renderer_pixel_test_backend.hpp"

#include <iostream>

namespace cgpui_test {
const char* pixel_backend_name(cgpui::RendererBackend backend) {
  switch (backend) {
    case cgpui::RendererBackend::vulkan: return "vulkan";
    case cgpui::RendererBackend::dx12: return "dx12";
    case cgpui::RendererBackend::metal: return "metal";
    default: return "unknown";
  }
}
cgpui::Result<cgpui::RendererBackend> parse_pixel_test_backend(std::string_view requested) {
  // Preserve the common tests' existing Vulkan default; this is not the dispatcher.
  if (requested.empty() || requested == "auto" || requested == "vulkan")
    return cgpui::RendererBackend::vulkan;
  if (requested == "dx12") return cgpui::RendererBackend::dx12;
  return std::unexpected(cgpui::Error{cgpui::ErrorCode::invalid_argument,
      "pixel test CGPUI_RENDERER must be auto, vulkan, or dx12 (both is runner-only)"});
}
cgpui::Result<void> validate_pixel_backend_info(
    cgpui::RendererBackend expected, const cgpui::RendererBackendInfo& info,
    bool require_software) {
  const bool known = expected == cgpui::RendererBackend::vulkan ||
                     expected == cgpui::RendererBackend::dx12;
  const auto selection = expected == cgpui::RendererBackend::dx12
      ? cgpui::RendererBackendSelection::dx12 : cgpui::RendererBackendSelection::vulkan;
  if (!known || info.backend != expected || info.selection != selection ||
      !info.fallback_reason.empty() || (require_software && !info.software_adapter)) {
    return std::unexpected(cgpui::Error{cgpui::ErrorCode::renderer_initialization_failed,
        std::string("pixel backend identity mismatch: requested=") + pixel_backend_name(expected) +
        " actual=" + pixel_backend_name(info.backend) + " selection=" +
        std::to_string(static_cast<int>(info.selection)) + " software=" +
        std::to_string(info.software_adapter) + " require_software=" +
        std::to_string(require_software) + " fallback=" + info.fallback_reason});
  }
  return {};
}
void report_pixel_test_error(std::string_view stage, const cgpui::Error& error) {
  std::cerr << "pixel test " << stage << ": code=" << static_cast<int>(error.code)
            << " " << error.message << '\n';
}
bool pixel_test_ok(const cgpui::Result<void>& result, std::string_view stage) {
  if (!result) report_pixel_test_error(stage, result.error());
  return result.has_value();
}
} // namespace cgpui_test
