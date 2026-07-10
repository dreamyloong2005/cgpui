#pragma once

#include "cgpui/renderer/renderer_frame_diagnostic_snapshot.hpp"

#include <cstddef>

namespace cgpui {

struct RendererFrameStatistics {
  RendererFrameDiagnostics work;
  RendererUploadByteCounts upload_bytes;
  RendererDrawCounts planned_draws;
  RendererDrawCounts submitted_draws;
  RendererDrawCounts dropped_draws;
  std::size_t dropped_resource_count = 0;
  std::size_t unsupported_primitive_count = 0;
  std::size_t missing_submission_resource_count = 0;
  RendererFrameTimings timings;
};

} // namespace cgpui
