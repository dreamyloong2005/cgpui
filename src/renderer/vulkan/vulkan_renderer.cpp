#include "../renderer_device_pixels_internal.hpp"
#include "vulkan_internal.hpp"
#include "vulkan_popup_support_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

class VulkanFrame final : public RenderFrame {
 public:
  explicit VulkanFrame(std::shared_ptr<VulkanRendererState> state)
      : state_(std::move(state)) {}

  void clear(Color color) override { clear_color_ = color; }
  void draw_rect(const SolidRect& rect) override {
    append_draw(rects_, rect, RendererPrimitiveKind::solid_rect);
  }
  void draw_rounded_rect(const RoundedRectDraw& rect) override {
    append_draw(rounded_rects_, rect, RendererPrimitiveKind::rounded_rect);
  }
  void draw_box_shadow(const BoxShadowDraw& shadow) override {
    append_draw(box_shadows_, shadow, RendererPrimitiveKind::box_shadow);
  }
  void draw_backdrop_blur(const BackdropBlurDraw& backdrop) override {
    if (!state_->backdrop_blur_support().supported) {
      RenderFrame::draw_backdrop_blur(backdrop);  // typed fallback: tint only
      return;
    }
    append_draw(backdrops_, backdrop, RendererPrimitiveKind::backdrop_blur);
  }
  void draw_text(const TextDraw& text) override {
    append_draw(text_draws_, text, RendererPrimitiveKind::text);
  }
  void draw_text_selection(const TextSelectionDraw& selection) override {
    append_draw(
        text_selections_, selection, RendererPrimitiveKind::text_selection);
  }
  void draw_text_caret(const TextCaretDraw& caret) override {
    append_draw(text_carets_, caret, RendererPrimitiveKind::text_caret);
  }
  void draw_image(const ImageDraw& image) override {
    append_draw(image_draws_, image, RendererPrimitiveKind::image);
  }
  void upload_image(const ImageAsset& image) override;
  void invalidate_image(ImageAssetId asset_id) override;
  Result<void> request_pixel_capture() override;
  Result<void> present() override;

 private:
  template <typename Draw>
  void append_draw(
      std::vector<Draw>& draws,
      const Draw& draw,
      RendererPrimitiveKind primitive_kind) {
    draw_order_.push_back(VulkanFrameDrawOrderEntry{
        .primitive_kind = primitive_kind,
        .command_index = draws.size(),
    });
    // Draws are logical pixels; the swapchain is device pixels.
    draws.push_back(renderer_to_device_pixels(draw, state_->scale()));
  }

  std::shared_ptr<VulkanRendererState> state_;
  Color clear_color_{.r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F};
  std::vector<SolidRect> rects_;
  std::vector<RoundedRectDraw> rounded_rects_;
  std::vector<BoxShadowDraw> box_shadows_;
  std::vector<BackdropBlurDraw> backdrops_;
  std::vector<TextDraw> text_draws_;
  std::vector<TextSelectionDraw> text_selections_;
  std::vector<TextCaretDraw> text_carets_;
  std::vector<ImageDraw> image_draws_;
  std::vector<ImageUploadBatch> image_uploads_;
  std::vector<ImageAssetId> image_invalidations_;
  std::vector<VulkanFrameDrawOrderEntry> draw_order_;
  bool capture_requested_ = false;
};

class VulkanRenderer final : public Renderer {
 public:
  explicit VulkanRenderer(std::shared_ptr<VulkanRendererState> state)
      : state_(std::move(state)) {}

  Result<void> resize(Size framebuffer_size, DpiScale scale) override {
    return state_->resize(framebuffer_size, scale);
  }

  Result<std::unique_ptr<RenderFrame>> begin_frame() override {
    return std::make_unique<VulkanFrame>(state_);
  }

  [[nodiscard]] const RendererFrameDiagnosticSnapshot*
  last_frame_diagnostic_snapshot() const override {
    return state_->last_frame_diagnostic_snapshot();
  }
  [[nodiscard]] const RendererFramePixels* last_frame_pixels() const override {
    return state_->last_frame_pixels();
  }
  [[nodiscard]] RendererSurfaceComposition surface_composition()
      const override {
    return state_->surface_composition();
  }
  [[nodiscard]] RendererEffectSupport backdrop_blur_support() const override {
    return state_->backdrop_blur_support();
  }
  [[nodiscard]] bool popup_windows_supported() const override {
    return vulkan_popup_windows_supported();
  }
  Result<RendererSurfaceComposition> set_transparent_surface(
      bool transparent) override {
    return state_->set_transparent_surface(transparent);
  }

 private:
  std::shared_ptr<VulkanRendererState> state_;
};

} // namespace

const RendererFrameDiagnosticSnapshot*
VulkanRendererState::last_frame_diagnostic_snapshot() const {
  return has_frame_diagnostic_snapshot_
             ? &last_frame_diagnostic_snapshot_
             : nullptr;
}

void VulkanFrame::upload_image(const ImageAsset& image) {
  const std::span<const ImageAsset> assets(&image, 1);
  std::vector<ImageUploadBatch> batches = vulkan_plan_image_uploads(assets);
  if (batches.empty()) {
    return;
  }
  const auto existing = std::ranges::find_if(
      image_uploads_,
      [&](const ImageUploadBatch& candidate) {
        return candidate.image.id == batches.front().image.id;
      });
  if (existing == image_uploads_.end()) {
    image_uploads_.push_back(std::move(batches.front()));
  } else {
    *existing = std::move(batches.front());
  }
}

void VulkanFrame::invalidate_image(ImageAssetId asset_id) {
  if (asset_id.value == 0 ||
      std::ranges::find(image_invalidations_, asset_id) !=
          image_invalidations_.end()) {
    return;
  }
  image_invalidations_.push_back(asset_id);
}

Result<void> VulkanFrame::request_pixel_capture() {
  if (auto result = state_->request_pixel_capture(); !result) {
    return result;
  }
  capture_requested_ = true;
  return {};
}

Result<void> VulkanFrame::present() {
  return state_->present_frame(
      clear_color_,
      draw_order_,
      rects_,
      rounded_rects_,
      box_shadows_,
      backdrops_,
      text_draws_,
      text_selections_,
      text_carets_,
      image_draws_,
      image_uploads_,
      image_invalidations_,
      capture_requested_);
}

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor& descriptor) {
  if (descriptor.framebuffer_size.width <= 0.0F ||
      descriptor.framebuffer_size.height <= 0.0F) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires a non-empty framebuffer",
    });
  }

  auto state = VulkanRendererState::create(descriptor);
  if (!state) {
    return std::unexpected(state.error());
  }

  return std::make_unique<VulkanRenderer>(std::move(*state));
}

} // namespace cgpui
