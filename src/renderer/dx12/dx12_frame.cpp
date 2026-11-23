#include "dx12_operation_internal.hpp"
#include "dx12_frame_internal.hpp"
#include "dx12_frame_recovery_internal.hpp"
#include "dx12_image_cache_internal.hpp"
#include "renderer_device_pixels_internal.hpp"
#include <cmath>

namespace cgpui {
Dx12Frame::Dx12Frame(std::shared_ptr<Dx12State> state) : state_(std::move(state)) {
  Dx12OperationGuard operation(*state_);
  generation_ = state_->info.device_generation;
}
Dx12Frame::~Dx12Frame() { release(); }
void Dx12Frame::release() {
  Dx12OperationGuard operation(*state_);
  if (!released_) { state_->frame_active = false; released_ = true; }
}
void Dx12Frame::clear(Color color) {
  Dx12OperationGuard operation(*state_);
  if (released_) return;
  if (!std::isfinite(color.r) || !std::isfinite(color.g) ||
      !std::isfinite(color.b) || !std::isfinite(color.a)) {
    error_ = Error{.code = ErrorCode::invalid_argument, .message = "DX12 clear color must be finite"};
    return;
  }
  clear_ = color;
}
void Dx12Frame::append(RectGeometry geometry) {
  Dx12OperationGuard operation(*state_);
  if (released_ || error_) return;
  if (!dx12_finite_geometry(geometry)) {
    error_ = Error{.code = ErrorCode::invalid_argument, .message = "DX12 geometry must be finite and indexable"};
    return;
  }
  draws_.push_back(std::move(geometry));
}
void Dx12Frame::append(Dx12TexturedDraw draw) {
  Dx12OperationGuard operation(*state_);
  if (released_ || error_) return;
  if (!dx12_finite_textured_draw(draw)) {
    error_ = Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 textured draw must have a valid texture and finite vertices"};
    return;
  }
  draws_.push_back(std::move(draw));
}
void Dx12Frame::draw_rect(const SolidRect& rect) {
  Dx12OperationGuard operation(*state_);
  if (released_ || error_) return;
  const auto device = renderer_to_device_pixels(rect, state_->scale);
  append(build_solid_rect_geometry(std::span{&device, 1}));
}
void Dx12Frame::draw_rounded_rect(const RoundedRectDraw& rect) {
  Dx12OperationGuard operation(*state_);
  if (released_ || error_) return;
  const auto device = renderer_to_device_pixels(rect, state_->scale);
  append(build_rounded_rect_geometry(std::span{&device, 1}));
}
void Dx12Frame::draw_text_selection(const TextSelectionDraw& selection) {
  Dx12OperationGuard operation(*state_);
  draw_rect(SolidRect{.rect = selection.rect, .color = selection.color,
      .clip_rect = selection.clip_rect, .clip_stack = selection.clip_stack,
      .composition_stack = selection.composition_stack, .metadata = selection.metadata});
}
void Dx12Frame::draw_text_caret(const TextCaretDraw& caret) {
  Dx12OperationGuard operation(*state_);
  draw_rect(SolidRect{.rect = caret.rect, .color = caret.color,
      .clip_rect = caret.clip_rect, .clip_stack = caret.clip_stack,
      .composition_stack = caret.composition_stack, .metadata = caret.metadata});
}
Result<void> Dx12Frame::request_pixel_capture() {
  Dx12OperationGuard operation(*state_);
  if (released_ || !state_->owner_alive) return std::unexpected(Error{
      .code = ErrorCode::invalid_argument, .message = "DX12 frame is no longer active"});
  capture_ = true;
  return {};
}
Result<void> Dx12Frame::present() {
  Dx12OperationGuard operation(*state_);
  if (released_) return std::unexpected(Error{.code = ErrorCode::invalid_argument,
      .message = "DX12 frame has already finished"});
  release();
  if (!state_->owner_alive) return std::unexpected(Error{.code = ErrorCode::invalid_argument,
      .message = "DX12 renderer was destroyed before its frame"});
  state_->pixels.reset();
  if (error_) return std::unexpected(*error_);
  if (auto ready = dx12_prepare_present(*state_, generation_); !ready) return ready;
  auto result = dx12_submit_frame(*state_, clear_, draws_, capture_);
  if (result) state_->image_cache->finish_frame();
  if (!result) return dx12_handle_frame_failure(*state_, std::move(result.error()));
  return result;
}
} // namespace cgpui
