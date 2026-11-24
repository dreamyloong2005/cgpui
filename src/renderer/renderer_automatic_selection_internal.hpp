#pragma once
#include "renderer_backend_selection_internal.hpp"

namespace cgpui::detail {
Result<std::unique_ptr<Renderer>> select_automatic_renderer(
    const RenderSurfaceDescriptor&, RendererFactory vulkan, RendererAttemptFactory dx12);
} // namespace cgpui::detail
