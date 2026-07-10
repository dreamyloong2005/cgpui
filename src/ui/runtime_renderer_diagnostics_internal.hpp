#pragma once

#include "cgpui/ui/runtime_diagnostics.hpp"

#include <optional>

namespace cgpui {

class Renderer;

void apply_runtime_renderer_frame_diagnostics(
    FrameStatistics& statistics,
    const RendererFrameDiagnosticSnapshot* snapshot);

[[nodiscard]] std::optional<RendererFrameDiagnosticSnapshot>
runtime_renderer_frame_diagnostic_snapshot(const Renderer& renderer);

} // namespace cgpui
