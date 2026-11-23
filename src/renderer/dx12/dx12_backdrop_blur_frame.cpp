#include "dx12_operation_internal.hpp"
#include "dx12_frame_internal.hpp"
#include "dx12_backdrop_blur_internal.hpp"
#include "renderer_device_pixels_internal.hpp"

namespace cgpui {
void Dx12Frame::draw_backdrop_blur(const BackdropBlurDraw& backdrop) {
  Dx12OperationGuard operation(*state_);
  if (released_ || error_) return;
  if (!state_->backdrop_blur->support.supported) {
    error_ = Error{.code = ErrorCode::unsupported_platform,
        .message = state_->backdrop_blur->support.reason};
    return;
  }
  const auto device = renderer_to_device_pixels(backdrop, state_->scale);
  if (!dx12_valid_backdrop_blur_input(device)) {
    error_ = Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 backdrop bounds, tint and clips must be finite"};
    return;
  }
  const PixelExtent extent{state_->width, state_->height};
  auto plan = plan_backdrop_blur(std::span{&device, 1}, extent);
  if (!dx12_valid_backdrop_blur_plan(plan, extent)) {
    error_ = Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 backdrop plan must have finite vertices and bounded passes"};
    return;
  }
  if (!plan.passes.empty()) draws_.push_back(std::move(plan));
}
} // namespace cgpui
