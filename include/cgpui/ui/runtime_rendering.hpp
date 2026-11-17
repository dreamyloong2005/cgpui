#pragma once

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    FrameStatistics* statistics = nullptr);
Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    DpiScale scale,
    FrameStatistics* statistics = nullptr);
Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    DpiScale scale,
    TextMeasurementCache* text_measurement_cache,
    FrameStatistics* statistics = nullptr);
// clear_color: alpha 0 lets a window backdrop show through the frame.
Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    DpiScale scale,
    TextMeasurementCache* text_measurement_cache,
    Color clear_color,
    FrameStatistics* statistics);
[[nodiscard]] int run_app(
    PlatformApplication& application,
    View& view,
    AppRendererFactory renderer_factory,
    AppRunnerOptions options = {});

} // namespace cgpui
