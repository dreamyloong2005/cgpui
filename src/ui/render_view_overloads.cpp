#include "ui_internal.hpp"

namespace cgpui {

Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    FrameStatistics* statistics) {
  return render_view(renderer, view, viewport_size, DpiScale{}, statistics);
}

Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    DpiScale scale,
    FrameStatistics* statistics) {
  return render_view(
      renderer,
      view,
      viewport_size,
      scale,
      nullptr,
      statistics);
}

Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    DpiScale scale,
    TextMeasurementCache* text_measurement_cache,
    FrameStatistics* statistics) {
  return render_view(
      renderer, view, viewport_size, scale, text_measurement_cache,
      runtime_default_clear_color, statistics);
}

Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    DpiScale scale,
    TextMeasurementCache* text_measurement_cache,
    Color clear_color,
    FrameStatistics* statistics) {
  return render_view(
      renderer, view, viewport_size, scale, text_measurement_cache,
      clear_color, nullptr, statistics);
}

Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    DpiScale scale,
    TextMeasurementCache* text_measurement_cache,
    Color clear_color,
    std::vector<WindowControlRegion>* window_control_regions,
    FrameStatistics* statistics) {
  return render_view(
      renderer, view, viewport_size, scale, text_measurement_cache,
      clear_color, window_control_regions, nullptr, statistics);
}

} // namespace cgpui
