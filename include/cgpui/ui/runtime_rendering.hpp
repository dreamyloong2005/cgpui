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
// window_control_regions: receives the client title bar regions painted in
// this frame (logical pixels), for PlatformWindow::set_window_control_regions.
Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    DpiScale scale,
    TextMeasurementCache* text_measurement_cache,
    Color clear_color,
    std::vector<WindowControlRegion>* window_control_regions,
    FrameStatistics* statistics);
// element_tree: the laid-out tree the view returned from render(). It is
// painted after View::paint, so View::paint draws beneath the elements.
// Its window control regions follow the View::paint ones.
Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    DpiScale scale,
    TextMeasurementCache* text_measurement_cache,
    Color clear_color,
    std::vector<WindowControlRegion>* window_control_regions,
    const ElementTree* element_tree,
    FrameStatistics* statistics);
[[nodiscard]] int run_app(
    PlatformApplication& application,
    View& view,
    AppRendererFactory renderer_factory,
    AppRunnerOptions options = {});

} // namespace cgpui
