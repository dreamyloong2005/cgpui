#include "dx12_operation_internal.hpp"
#include "dx12_frame_internal.hpp"
#include "dx12_text_draws_internal.hpp"
#include "renderer_device_pixels_internal.hpp"

namespace cgpui {

void Dx12Frame::draw_text(const TextDraw& text) {
  Dx12OperationGuard operation(*state_);
  if (released_ || error_) return;
  auto prepared = dx12_build_text_draws(*state_, renderer_to_device_pixels(text, state_->scale));
  if (!prepared) {
    error_ = prepared.error();
    return;
  }
  for (auto& draw : *prepared) append(std::move(draw));
}

} // namespace cgpui
