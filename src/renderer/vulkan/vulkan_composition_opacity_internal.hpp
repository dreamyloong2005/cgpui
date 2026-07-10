#pragma once

#include "vulkan_platform_internal.hpp"

namespace cgpui {

[[nodiscard]] float vulkan_resolve_composed_opacity(
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack);
[[nodiscard]] Color vulkan_apply_composed_opacity(
    Color color,
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack);

} // namespace cgpui
