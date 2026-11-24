#include "renderer_backend_selection_internal.hpp"
#include "renderer_selection_metadata_internal.hpp"
#include "renderer_automatic_selection_internal.hpp"

#include <utility>

namespace cgpui::detail {

Result<RendererBackendSelection> parse_renderer_backend_selection(
    std::string_view preference) {
  if (preference.empty() || preference == "auto") {
    return RendererBackendSelection::automatic;
  }
  if (preference == "vulkan") return RendererBackendSelection::vulkan;
  if (preference == "dx12") return RendererBackendSelection::dx12;
  return std::unexpected(Error{
      .code = ErrorCode::invalid_argument,
      .message = "CGPUI_RENDERER must be auto, vulkan, or dx12",
  });
}

Result<std::unique_ptr<Renderer>> select_renderer_backend(
    const RenderSurfaceDescriptor& descriptor, std::string_view preference,
    RendererFactory vulkan_factory, RendererFactory dx12_factory,
    RendererAttemptFactory automatic_attempt) {
  const auto selection = parse_renderer_backend_selection(preference);
  if (!selection) return std::unexpected(selection.error());
  if (*selection == RendererBackendSelection::automatic)
    return select_automatic_renderer(descriptor, vulkan_factory, automatic_attempt);
  // Only explicit requests reach named factories; they never fall back.
  const auto factory = *selection == RendererBackendSelection::dx12
      ? dx12_factory : vulkan_factory;
  if (!factory) return std::unexpected(Error{
      .code = ErrorCode::renderer_initialization_failed,
      .message = "requested renderer backend is not available",
  });
  auto renderer = factory(descriptor);
  if (!renderer) return std::unexpected(renderer.error());
  if (!*renderer) return std::unexpected(Error{
      .code = ErrorCode::renderer_initialization_failed,
      .message = "requested renderer factory returned an empty renderer",
  });
  return renderer_with_selection_metadata(std::move(*renderer), *selection);
}

} // namespace cgpui::detail
