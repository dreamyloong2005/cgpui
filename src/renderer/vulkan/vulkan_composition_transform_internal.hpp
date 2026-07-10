#pragma once

#include "vulkan_platform_internal.hpp"

namespace cgpui {

[[nodiscard]] AffineTransform vulkan_resolve_composed_transform(
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack);
[[nodiscard]] Point vulkan_transform_point(
    Point point,
    AffineTransform transform);
[[nodiscard]] Point vulkan_apply_composed_transform(
    Point point,
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack);

} // namespace cgpui
