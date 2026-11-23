#include "dx12_runtime_recovery_scene_test_support.hpp"

#include "cgpui/ui/runtime_context.hpp"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "dx12_test_support.hpp"

#include <cmath>

namespace cgpui_test {

Dx12RuntimeRecoveryScene::Dx12RuntimeRecoveryScene(
    cgpui::ImageAssetId id, Dx12RuntimeSceneState& state)
    : image_{.id = id, .logical_size = {2, 2},
        .bitmap = {.width = 2, .height = 2, .stride = 8,
            .pixels = {255, 0, 0, 255, 0, 255, 0, 255,
                       0, 0, 255, 255, 255, 255, 255, 255}}}, state_(state) {}

void Dx12RuntimeRecoveryScene::paint(cgpui::PaintList& list, cgpui::Size viewport) {
  ++state_.paints;
  if (state_.upload_intents == 0) {
    // PaintList submits this pointer AFTER paint returns: image_ is View-owned.
    list.upload_image(image_);
    ++state_.upload_intents;
  }
  list.fill_rect({{0, 0}, viewport}, {0, 0, 0, 1});
  list.draw_image({{12, 12}, {48, 48}}, cgpui::describe_image_asset(image_),
      std::nullopt, std::nullopt, cgpui::ImageSamplingMode::nearest);
  list.fill_text({{80, 64}, {80, 48}}, {1, 1, 1, 1}, "M", {.family = "Segoe UI"}, 32);
  list.fill_rect({{80, 12}, {32, 32}}, state_.epoch == 0
      ? cgpui::Color{1, 0, 1, 1} : cgpui::Color{0, 1, 1, 1});
}

bool dx12_runtime_scene_pixels(const cgpui::WindowRuntimeContext& context,
    unsigned epoch, std::vector<std::uint8_t>& glyph_reference) {
  const auto* pixels = context.renderer.last_frame_pixels();
  const float scale = context.scale.value;
  if (!pixels || !pixels->valid() || !std::isfinite(scale) || scale <= 0 ||
      context.viewport_size.width < 180 || context.viewport_size.height < 120) return false;
  const auto sample = [scale](float logical) {
    return static_cast<unsigned>(std::floor(logical * scale));
  };
  if (!dx12_pixel(pixels, sample(24), sample(24), {255, 0, 0, 255}) ||
      !dx12_pixel(pixels, sample(48), sample(24), {0, 255, 0, 255}) ||
      !dx12_pixel(pixels, sample(24), sample(48), {0, 0, 255, 255}) ||
      !dx12_pixel(pixels, sample(48), sample(48), {255, 255, 255, 255}) ||
      !dx12_pixel(pixels, sample(96), sample(28), epoch == 0
          ? std::array<unsigned char, 4>{255, 0, 255, 255}
          : std::array<unsigned char, 4>{0, 255, 255, 255})) return false;

  std::vector<std::uint8_t> glyph;
  unsigned ink = 0;
  for (unsigned y = sample(64); y < sample(116); ++y) {
    for (unsigned x = sample(76); x < sample(168); ++x) {
      const auto pixel = pixels->pixel_rgba8(x, y);
      if (!pixel) return false;
      glyph.insert(glyph.end(), pixel->begin(), pixel->end());
      if ((*pixel)[0] > 16 && (*pixel)[0] == (*pixel)[1] &&
          (*pixel)[1] == (*pixel)[2] && (*pixel)[3] == 255) ++ink;
    }
  }
  if (ink <= 8) return false; // An equally blank capture cannot pass.
  if (glyph_reference.empty()) glyph_reference = glyph;
  return glyph == glyph_reference;
}

} // namespace cgpui_test
