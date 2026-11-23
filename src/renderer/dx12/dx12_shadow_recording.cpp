#include "dx12_shadow_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_indexed_draw_internal.hpp"

namespace cgpui {
Result<void> dx12_record_shadow_geometry(Dx12State& state, const BoxShadowGeometry& geometry) {
  if (geometry.vertices.empty() || geometry.indices.empty()) return {};
  if (auto pipeline = dx12_create_shadow_pipeline(state); !pipeline) return pipeline;
  return dx12_record_indexed_draw(state, *state.shadow_pipeline.Get(),
      std::as_bytes(std::span{geometry.vertices}), sizeof(BoxShadowVertex), geometry.indices, geometry.draws);
}
} // namespace cgpui
