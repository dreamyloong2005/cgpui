#include "dx12_operation_internal.hpp"
#include "dx12_frame_internal.hpp"
#include "dx12_shadow_internal.hpp"
#include "renderer_device_pixels_internal.hpp"

namespace cgpui {
void Dx12Frame::draw_box_shadow(const BoxShadowDraw& shadow) {
  Dx12OperationGuard operation(*state_);
  if (released_ || error_) return;
  const auto device = renderer_to_device_pixels(shadow, state_->scale);
  auto geometry = build_box_shadow_geometry(std::span{&device, 1});
  if (!dx12_finite_shadow_geometry(geometry)) {
    error_ = Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 box shadow geometry must be finite and indexable"};
    return;
  }
  draws_.push_back(std::move(geometry));
}
} // namespace cgpui
