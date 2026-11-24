#pragma once

#include "cgpui/renderer/renderer_backend_info.hpp"
#include "renderer_initialization_attempt_internal.hpp"
#include "cgpui/renderer/renderer_frame.hpp"

#include <string_view>

namespace cgpui::detail {

using RendererFactory = Result<std::unique_ptr<Renderer>> (*)(
    const RenderSurfaceDescriptor&);

Result<RendererBackendSelection> parse_renderer_backend_selection(
    std::string_view preference);

// GPU-free policy seam. Automatic selection requires a classified DX12 attempt;
// only a proved reusable surface permits initialization-only Vulkan fallback.
Result<std::unique_ptr<Renderer>> select_renderer_backend(
    const RenderSurfaceDescriptor& descriptor, std::string_view preference,
    RendererFactory vulkan_factory, RendererFactory dx12_factory,
    RendererAttemptFactory automatic_attempt = nullptr);

} // namespace cgpui::detail
