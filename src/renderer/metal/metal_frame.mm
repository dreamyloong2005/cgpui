#include "metal_renderer_internal.hpp"

#include <algorithm>
#include <chrono>

namespace cgpui {
namespace {

using Clock = std::chrono::steady_clock;

std::uint64_t elapsed_nanoseconds(Clock::time_point started) {
  return static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          Clock::now() - started).count());
}

class MetalFrame final : public RenderFrame {
 public:
  MetalFrame(
      std::shared_ptr<MetalRendererState> state,
      MetalAcquiredFrame resources)
      : state_(std::move(state)), resources_(std::move(resources)) {}

  ~MetalFrame() override {
    if (resources_.pacing_slot) state_->pacing.release();
  }

  void clear(Color color) override { clear_color_ = color; }
  void draw_rect(const SolidRect& rect) override { pending_rect_ = rect; }
  Result<void> present() override {
    if (presented_ || !resources_.pacing_slot) {
      return std::unexpected(Error{
          .code = ErrorCode::invalid_argument,
          .message = "Metal frame can only be presented once"});
    }

    RendererFrameTimings timings = resources_.timings;
    const auto recording_started = Clock::now();
    MTLRenderPassDescriptor* pass =
        [MTLRenderPassDescriptor renderPassDescriptor];
    pass.colorAttachments[0].texture = resources_.drawable.texture;
    pass.colorAttachments[0].loadAction = MTLLoadActionClear;
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;
    pass.colorAttachments[0].clearColor = MTLClearColorMake(
        std::clamp(clear_color_.r, 0.0F, 1.0F),
        std::clamp(clear_color_.g, 0.0F, 1.0F),
        std::clamp(clear_color_.b, 0.0F, 1.0F),
        std::clamp(clear_color_.a, 0.0F, 1.0F));
    id<MTLRenderCommandEncoder> encoder =
        [resources_.command_buffer renderCommandEncoderWithDescriptor:pass];
    if (encoder == nil) {
      return std::unexpected(Error{
          .code = ErrorCode::frame_acquisition_failed,
          .message = "Metal render command encoder creation failed"});
    }
    [encoder endEncoding];
    renderer_add_frame_timing(
        timings,
        RendererFrameTimingStage::command_recording,
        elapsed_nanoseconds(recording_started));

    const auto submission_started = Clock::now();
    std::shared_ptr<MetalRendererState> state = state_;
    [resources_.command_buffer addCompletedHandler:^(id<MTLCommandBuffer>) {
      state->pacing.release();
    }];
    [resources_.command_buffer presentDrawable:resources_.drawable];
    [resources_.command_buffer commit];
    renderer_add_frame_timing(
        timings,
        RendererFrameTimingStage::queue_submission,
        elapsed_nanoseconds(submission_started));
    state_->record_clear_frame(timings);
    resources_.pacing_slot = false;
    presented_ = true;
    return {};
  }

 private:
  std::shared_ptr<MetalRendererState> state_;
  MetalAcquiredFrame resources_;
  Color clear_color_{.r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F};
  SolidRect pending_rect_{};
  bool presented_ = false;
};

}  // namespace

Result<MetalAcquiredFrame> MetalRendererState::acquire_frame() {
  const auto acquisition_started = Clock::now();
  if (!pacing.try_acquire()) {
    return std::unexpected(Error{
        .code = ErrorCode::frame_acquisition_failed,
        .message = "Metal frame pacing limit reached"});
  }

  id<CAMetalDrawable> drawable = [layer nextDrawable];
  if (drawable == nil) {
    pacing.release();
    return std::unexpected(Error{
        .code = ErrorCode::frame_acquisition_failed,
        .message = "CAMetalLayer did not provide a drawable"});
  }
  id<MTLCommandBuffer> command_buffer = [command_queue commandBuffer];
  if (command_buffer == nil) {
    pacing.release();
    return std::unexpected(Error{
        .code = ErrorCode::frame_acquisition_failed,
        .message = "Metal command buffer creation failed"});
  }
  RendererFrameTimings timings;
  renderer_add_frame_timing(
      timings,
      RendererFrameTimingStage::image_acquisition,
      elapsed_nanoseconds(acquisition_started));
  return MetalAcquiredFrame{
      .drawable = drawable,
      .command_buffer = command_buffer,
      .frame_id = next_frame_id.fetch_add(1, std::memory_order_relaxed),
      .timings = timings,
      .pacing_slot = true};
}

Result<std::unique_ptr<RenderFrame>> make_metal_frame(
    std::shared_ptr<MetalRendererState> state) {
  auto resources = state->acquire_frame();
  if (!resources) return std::unexpected(resources.error());
  return std::make_unique<MetalFrame>(std::move(state), std::move(*resources));
}

}  // namespace cgpui
