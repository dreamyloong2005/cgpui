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
  void draw_rect(const SolidRect& rect) override {
    commands_.draw_order.push_back({
        .primitive_kind = RendererPrimitiveKind::solid_rect,
        .command_index = commands_.solid_rects.size()});
    commands_.solid_rects.push_back(rect);
  }
  void draw_rounded_rect(const RoundedRectDraw& rect) override {
    commands_.draw_order.push_back({
        .primitive_kind = RendererPrimitiveKind::rounded_rect,
        .command_index = commands_.rounded_rects.size()});
    commands_.rounded_rects.push_back(rect);
  }
  void draw_text(const TextDraw& text) override {
    commands_.draw_order.push_back({
        .primitive_kind = RendererPrimitiveKind::text,
        .command_index = commands_.text_draws.size()});
    commands_.text_draws.push_back(text);
  }
  void draw_text_selection(const TextSelectionDraw& selection) override {
    commands_.draw_order.push_back({
        .primitive_kind = RendererPrimitiveKind::text_selection,
        .command_index = commands_.selections.size()});
    commands_.selections.push_back(selection);
  }
  void draw_text_caret(const TextCaretDraw& caret) override {
    commands_.draw_order.push_back({
        .primitive_kind = RendererPrimitiveKind::text_caret,
        .command_index = commands_.carets.size()});
    commands_.carets.push_back(caret);
  }
  void upload_image(const ImageAsset& image) override {
    commands_.image_uploads.push_back(image);
  }
  void invalidate_image(ImageAssetId asset_id) override {
    commands_.image_invalidations.push_back(asset_id);
  }
  void draw_image(const ImageDraw& image) override {
    commands_.draw_order.push_back({
        .primitive_kind = RendererPrimitiveKind::image,
        .command_index = commands_.image_draws.size()});
    commands_.image_draws.push_back(image);
  }
  Result<void> request_pixel_capture() override {
    commands_.capture_requested = true;
    return {};
  }
  Result<void> present() override {
    if (presented_ || !resources_.pacing_slot) {
      return std::unexpected(Error{
          .code = ErrorCode::invalid_argument,
          .message = "Metal frame can only be presented once"});
    }

    auto image_upload_bytes = metal_upload_images(*state_, commands_);
    if (!image_upload_bytes) return std::unexpected(image_upload_bytes.error());

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
    std::size_t upload_byte_count = *image_upload_bytes;
    for (const MetalFrameDrawOrderEntry entry : commands_.draw_order) {
      Result<void> encoded;
      switch (entry.primitive_kind) {
        case RendererPrimitiveKind::solid_rect:
          encoded = metal_encode_solid_rect(
              encoder, *state_, commands_.solid_rects[entry.command_index]);
          break;
        case RendererPrimitiveKind::rounded_rect:
          encoded = metal_encode_rounded_rect(
              encoder, *state_, commands_.rounded_rects[entry.command_index]);
          break;
        case RendererPrimitiveKind::text: {
          auto text_upload = metal_encode_text(
              encoder, *state_, commands_.text_draws[entry.command_index]);
          if (!text_upload) encoded = std::unexpected(text_upload.error());
          else upload_byte_count += *text_upload;
          break;
        }
        case RendererPrimitiveKind::text_selection: {
          const TextSelectionDraw& selection =
              commands_.selections[entry.command_index];
          encoded = metal_encode_solid_rect(
              encoder,
              *state_,
              SolidRect{
                  .rect = selection.rect,
                  .color = selection.color,
                  .clip_rect = selection.clip_rect,
                  .clip_stack = selection.clip_stack,
                  .composition_stack = selection.composition_stack,
                  .metadata = selection.metadata});
          break;
        }
        case RendererPrimitiveKind::text_caret: {
          const TextCaretDraw& caret = commands_.carets[entry.command_index];
          encoded = metal_encode_solid_rect(
              encoder,
              *state_,
              SolidRect{
                  .rect = caret.rect,
                  .color = caret.color,
                  .clip_rect = caret.clip_rect,
                  .clip_stack = caret.clip_stack,
                  .composition_stack = caret.composition_stack,
                  .metadata = caret.metadata});
          break;
        }
        case RendererPrimitiveKind::image:
          encoded = metal_encode_image(
              encoder, *state_, commands_.image_draws[entry.command_index]);
          break;
      }
      if (!encoded) {
        [encoder endEncoding];
        return std::unexpected(encoded.error());
      }
    }
    [encoder endEncoding];
    renderer_add_frame_timing(
        timings,
        RendererFrameTimingStage::command_recording,
        elapsed_nanoseconds(recording_started));

    std::size_t capture_row_byte_count = 0;
    id<MTLBuffer> capture_buffer = nil;
    if (commands_.capture_requested) {
      capture_buffer = metal_encode_pixel_capture(
          *state_,
          resources_.command_buffer,
          resources_.drawable.texture,
          capture_row_byte_count);
      if (capture_buffer == nil) {
        return std::unexpected(Error{
            .code = ErrorCode::frame_acquisition_failed,
            .message = "Metal pixel capture buffer creation failed"});
      }
    }

    const auto submission_started = Clock::now();
    std::shared_ptr<MetalRendererState> state = state_;
    [resources_.command_buffer addCompletedHandler:^(id<MTLCommandBuffer>) {
      state->pacing.release();
    }];
    [resources_.command_buffer presentDrawable:resources_.drawable];
    [resources_.command_buffer commit];
    resources_.pacing_slot = false;
    presented_ = true;
    if (commands_.capture_requested) {
      [resources_.command_buffer waitUntilCompleted];
      if (resources_.command_buffer.status != MTLCommandBufferStatusCompleted) {
        return std::unexpected(Error{
            .code = ErrorCode::frame_acquisition_failed,
            .message = "Metal pixel capture command buffer failed"});
      }
      state_->store_pixels(
          capture_buffer,
          static_cast<std::uint32_t>(resources_.drawable.texture.width),
          static_cast<std::uint32_t>(resources_.drawable.texture.height),
          capture_row_byte_count);
    }
    renderer_add_frame_timing(
        timings,
        RendererFrameTimingStage::queue_submission,
        elapsed_nanoseconds(submission_started));
    const std::vector<RendererCommandBatch> batches =
        metal_build_command_batches(commands_);
    state_->record_frame(
        timings,
        commands_,
        std::max<std::size_t>(1, batches.size()),
        upload_byte_count - *image_upload_bytes,
        *image_upload_bytes);
    return {};
  }

 private:
  std::shared_ptr<MetalRendererState> state_;
  MetalAcquiredFrame resources_;
  MetalFrameCommands commands_;
  Color clear_color_{.r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F};
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
