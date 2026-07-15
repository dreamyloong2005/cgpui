#include "metal_renderer_internal.hpp"

namespace cgpui {

void MetalRendererState::record_clear_frame(RendererFrameTimings timings) {
  const RendererFrameWork work{
      .batch_count = 1,
      .command_count = 1,
      .upload_byte_count = 0,
      .draw_count = 0};
  const std::scoped_lock lock(snapshot_mutex);
  last_snapshot = RendererFrameDiagnosticSnapshot{
      .work = compare_renderer_frame_work(work, work),
      .timings = timings};
  has_snapshot = true;
}

const RendererFrameDiagnosticSnapshot* MetalRendererState::snapshot() const {
  const std::scoped_lock lock(snapshot_mutex);
  return has_snapshot ? &last_snapshot : nullptr;
}

}  // namespace cgpui
