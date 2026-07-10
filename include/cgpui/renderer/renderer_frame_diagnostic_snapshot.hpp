#pragma once

#include "cgpui/renderer/renderer_dropped_resource_diagnostics.hpp"
#include "cgpui/renderer/renderer_frame_timing_diagnostics.hpp"

namespace cgpui {

struct RendererFrameDiagnosticSnapshot {
  RendererFrameDiagnostics work;
  RendererUploadByteCounts upload_bytes;
  RendererDrawCounts planned_draws;
  RendererDrawCounts submitted_draws;
  RendererDroppedResourceDiagnostics dropped_resources;
  RendererFrameTimings timings;
};

} // namespace cgpui
