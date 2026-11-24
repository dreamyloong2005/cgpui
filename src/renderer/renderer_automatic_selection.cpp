#include "renderer_automatic_selection_internal.hpp"
#include "renderer_selection_metadata_internal.hpp"
#include <utility>

namespace cgpui::detail {
namespace {
Error invalid_factory(std::string message) {
  return {ErrorCode::renderer_initialization_failed, std::move(message)};
}
bool eligible(const RendererInitializationFailure& failure) {
  return failure.same_surface_reusable &&
      (failure.error.code == ErrorCode::renderer_initialization_failed ||
       failure.error.code == ErrorCode::unsupported_platform);
}
} // namespace
Result<std::unique_ptr<Renderer>> select_automatic_renderer(
    const RenderSurfaceDescriptor& descriptor, RendererFactory vulkan, RendererAttemptFactory dx12) {
  if (!dx12) return std::unexpected(invalid_factory("automatic DX12 attempt is not available"));
  auto preferred = dx12(descriptor);
  if (preferred) {
    if (!*preferred || (*preferred)->backend_info().backend != RendererBackend::dx12)
      return std::unexpected(invalid_factory("automatic DX12 factory returned an invalid renderer"));
    return renderer_with_selection_metadata(std::move(*preferred), RendererBackendSelection::automatic);
  }
  const auto& failure = preferred.error();
  if (!eligible(failure)) return std::unexpected(failure.error);
  const std::string reason = "DX12 initialization failed: " + failure.error.message;
  if (!vulkan) return std::unexpected(invalid_factory(reason + "; Vulkan factory is unavailable"));
  auto fallback = vulkan(descriptor);
  if (!fallback) return std::unexpected(Error{fallback.error().code,
      reason + "; Vulkan initialization failed: " + fallback.error().message});
  if (!*fallback || (*fallback)->backend_info().backend != RendererBackend::vulkan)
    return std::unexpected(invalid_factory(reason + "; Vulkan factory returned an invalid renderer"));
  return renderer_with_selection_metadata(std::move(*fallback), RendererBackendSelection::automatic, reason);
}
} // namespace cgpui::detail
