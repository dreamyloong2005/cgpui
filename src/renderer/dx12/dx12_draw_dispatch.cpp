#include "dx12_frame_internal.hpp"
#include "dx12_shadow_internal.hpp"

namespace cgpui {
namespace {
struct DrawRecorder {
  Dx12State& state;
  Result<void> operator()(const RectGeometry& geometry) const {
    return dx12_record_geometry(state, geometry);
  }
  Result<void> operator()(const Dx12TexturedDraw& draw) const {
    return dx12_record_textured_draw(state, draw);
  }
  Result<void> operator()(const BoxShadowGeometry& geometry) const {
    return dx12_record_shadow_geometry(state, geometry);
  }
};
} // namespace

Result<void> dx12_record_draw(Dx12State& state, const Dx12DrawCommand& draw) {
  return std::visit(DrawRecorder{state}, draw);
}
} // namespace cgpui
