#include "cgpui/ui/text_measurement.hpp"

#include "text_shaping_native_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

float text_line_metrics_baseline(float line_height) {
  return line_height * 0.8F;
}

TextLineMetrics line_metrics_from(float ascent, float descent, float scale) {
  return TextLineMetrics{
      .ascent = ascent,
      .descent = descent,
      .leading = 0.0F,
      .line_height = ascent + descent,
      .baseline = ascent,
      .device_ascent = ascent * scale,
      .device_descent = descent * scale,
      .device_leading = 0.0F,
      .device_line_height = (ascent + descent) * scale,
      .device_baseline = ascent * scale,
  };
}

} // namespace

// The line box holds the tallest face used on the run; each face's line
// gap is split evenly above and below (CSS half-leading).
TextLineMetrics native_text_line_metrics(const TextShapeRun& run) {
  const auto half_gap = [](const TextFontMetrics& metrics) {
    return std::max(0.0F, metrics.line_gap) * 0.5F;
  };
  float ascent = run.font_metrics.ascent + half_gap(run.font_metrics);
  float descent = run.font_metrics.descent + half_gap(run.font_metrics);
  for (const TextFontFallbackRun& font_run : run.font_runs) {
    ascent = std::max(ascent, font_run.metrics.ascent + half_gap(font_run.metrics));
    descent =
        std::max(descent, font_run.metrics.descent + half_gap(font_run.metrics));
  }
  return line_metrics_from(ascent, descent, normalized_scale(run.scale));
}

TextLineMetrics text_line_metrics_for_shape_run(const TextShapeRun& run) {
  if (run.font_metrics.ascent > 0.0F) {
    return native_text_line_metrics(run);
  }
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
