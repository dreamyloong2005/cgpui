#pragma once

#include "dx12_state_internal.hpp"
#include "dx12_draw_commands_internal.hpp"

namespace cgpui {
Result<void> dx12_submit_frame(Dx12State& state, Color clear,
                              std::span<const Dx12DrawCommand> draws, bool capture,
                              std::size_t command_count);
Result<void> dx12_record_geometry(Dx12State& state, const RectGeometry& geometry);
bool dx12_finite_geometry(const RectGeometry& geometry);

class Dx12Frame final : public RenderFrame {
 public:
  explicit Dx12Frame(std::shared_ptr<Dx12State> state);
  ~Dx12Frame() override;
  void clear(Color color) override;
  void draw_rect(const SolidRect& rect) override;
  void draw_rounded_rect(const RoundedRectDraw& rect) override;
  void draw_box_shadow(const BoxShadowDraw&) override;
  void draw_backdrop_blur(const BackdropBlurDraw&) override;
  void draw_text(const TextDraw&) override;
  void draw_text_selection(const TextSelectionDraw&) override;
  void draw_text_caret(const TextCaretDraw&) override;
  void upload_image(const ImageAsset&) override;
  void invalidate_image(ImageAssetId) override;
  void draw_image(const ImageDraw&) override;
  Result<void> request_pixel_capture() override;
  Result<void> present() override;

 private:
  void append(RectGeometry geometry, RendererPrimitiveKind kind);
  void append(Dx12TexturedDraw draw, RendererPrimitiveKind kind,
              std::size_t command_index, std::size_t resource_offset = 0);
  void append_rect(const SolidRect& rect, RendererPrimitiveKind kind);
  void release();
  std::shared_ptr<Dx12State> state_;
  std::uint64_t generation_ = 0;
  Color clear_{0, 0, 0, 1};
  std::vector<Dx12DrawCommand> draws_;
  std::size_t command_count_ = 0;
  std::optional<Error> error_;
  bool capture_ = false;
  bool released_ = false;
};
} // namespace cgpui
