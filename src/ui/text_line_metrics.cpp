#include "cgpui/ui/text_measurement.hpp"

#include <algorithm>

namespace cgpui {
namespace {

float text_line_metrics_baseline(float line_height) {
  return line_height * 0.8F;
}

} // namespace

TextLineMetrics text_line_metrics_for_shape_run(const TextShapeRun& run) {
  const float baseline = text_line_metrics_baseline(run.line_height);
  const float device_baseline =
      text_line_metrics_baseline(run.device_line_height);
  return TextLineMetrics{
      .ascent = baseline,
      .descent = std::max(0.0F, run.line_height - baseline),
      .leading = 0.0F,
      .line_height = run.line_height,
      .baseline = baseline,
      .device_ascent = device_baseline,
      .device_descent =
          std::max(0.0F, run.device_line_height - device_baseline),
      .device_leading = 0.0F,
      .device_line_height = run.device_line_height,
      .device_baseline = device_baseline,
  };
}

} // namespace cgpui
