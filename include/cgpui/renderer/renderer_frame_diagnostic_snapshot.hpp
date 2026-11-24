#pragma once

#include "cgpui/renderer/renderer_dropped_resource_diagnostics.hpp"
#include "cgpui/renderer/renderer_effect_draws.hpp"
#include "cgpui/renderer/renderer_frame_timing_diagnostics.hpp"
#include <cstddef>
#include <optional>

namespace cgpui {

struct RendererFrameDiagnosticSnapshot {
  RendererFrameDiagnostics work;
  RendererUploadByteCounts upload_bytes;
  RendererDrawCounts planned_draws;
  RendererDrawCounts submitted_draws;
  RendererDroppedResourceDiagnostics dropped_resources;
  RendererFrameTimings timings;
  RendererBackdropBlurDiagnostics backdrop_blur;
  // Unique live immutable RGBA image CPU payload, sampled after successful cache
  // aging. Unreported/unavailable is not known zero; excludes glyph/native memory.
  std::optional<std::size_t> retained_image_byte_count;
  bool retained_image_byte_count_saturated = false;
};

} // namespace cgpui
