#pragma once

#include "cgpui/renderer/renderer_commands.hpp"

namespace cgpui {

[[nodiscard]] AffineTransform resolve_composed_transform(
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack);
[[nodiscard]] Point transform_point(
    Point point,
    AffineTransform transform);
[[nodiscard]] Point apply_composed_transform(
    Point point,
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack);

} // namespace cgpui
