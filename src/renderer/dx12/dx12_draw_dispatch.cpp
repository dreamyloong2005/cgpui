#include "dx12_frame_internal.hpp"
#include "dx12_shadow_internal.hpp"
#include "dx12_backdrop_blur_internal.hpp"

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
  Result<void> operator()(const Dx12MissingImageDraw&) const { return {}; }
  Result<void> operator()(const Dx12PendingImageDraw&) const {
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 native draw requires a resolved image request"});
  }
  Result<void> operator()(const BackdropBlurPlan& plan) const {
    return dx12_record_backdrop_blur(state, plan);
  }
};
} // namespace

Result<void> dx12_record_draw(Dx12State& state, const Dx12DrawCommand& draw) {
  return std::visit(DrawRecorder{state}, draw.payload);
}
} // namespace cgpui
