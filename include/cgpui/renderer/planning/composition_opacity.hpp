#pragma once

#include "cgpui/renderer/renderer_commands.hpp"

namespace cgpui {

[[nodiscard]] float resolve_composed_opacity(
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack);
[[nodiscard]] Color apply_composed_opacity(
    Color color,
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack);

} // namespace cgpui
