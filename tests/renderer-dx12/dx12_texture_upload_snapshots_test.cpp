#include "dx12_texture_upload_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool image_tile(const RendererFramePixels* pixels, unsigned left, unsigned top, bool original) {
  const std::array<std::array<unsigned char, 4>, 4> colors{{
      {255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {255, 255, 255, 255}}};
  for (unsigned y = 0; y != 16; ++y) for (unsigned x = 0; x != 16; ++x) {
    const auto expected = original ? colors[(y / 8) * 2 + x / 8]
                                   : std::array<unsigned char, 4>{0, 255, 0, 255};
    if (!dx12_pixel(pixels, left + x, top + y, expected)) return false;
  }
  return true;
}
} // namespace

int main() {
  Dx12Window window;
  auto state = texture_upload_state(&window);
  if (!state) return dx12_fail(1, "texture snapshot mixed fixture unavailable");
  auto original = diagnostic_image();
  auto replacement = original;
  for (unsigned y = 0; y != 2; ++y) for (unsigned x = 0; x != 2; ++x) {
    const auto offset = y * replacement.bitmap.stride + x * 4;
    replacement.bitmap.pixels[offset] = replacement.bitmap.pixels[offset + 2] = 0;
    replacement.bitmap.pixels[offset + 1] = replacement.bitmap.pixels[offset + 3] = 255;
  }
  const auto asset = describe_image_asset(original);
  for (unsigned pass = 0; pass != 2; ++pass) {
    state->frame_active = true;
    Dx12Frame frame(state);
    frame.clear({0, 0, 0, 1});
    if (!pass) frame.upload_image(original);
    frame.draw_image({.bounds = {{0, 0}, {16, 16}}, .asset = asset, .sampling = ImageSamplingMode::nearest});
    if (!pass) {
      // A later same-id upload and mutations of caller bytes cannot rewrite
      // the first draw's immutable snapshot or share its staging slice.
      original.bitmap.pixels.assign(original.bitmap.pixels.size(), 0);
      frame.upload_image(replacement);
    }
    for (const float y : {0.0F, 24.0F})
      frame.draw_image({.bounds = {{24, y}, {16, 16}}, .asset = asset, .sampling = ImageSamplingMode::nearest});
    for (const float y : {24.0F, 44.0F}) frame.draw_text(diagnostic_text("M", 16, {0, y}));
    if (!frame.request_pixel_capture() || !frame.present() || !state->pixels ||
        !image_tile(&*state->pixels, 0, 0, !pass) || !image_tile(&*state->pixels, 24, 0, false) ||
        !image_tile(&*state->pixels, 24, 24, false) || !diagnostic_ink(&*state->pixels, 0, 24, 20, 42) ||
        !diagnostic_ink(&*state->pixels, 0, 44, 20, 64))
      return dx12_fail(2, "mixed image/glyph staging changed snapshot pixels or text coverage");
    const auto& diagnostic = *state->diagnostics;
    const auto stats = state->texture_uploads->stats();
    if (!diagnostic_work(&diagnostic, 5, 5, 5) || diagnostic.submitted_draws.image_count != 3 ||
        diagnostic.submitted_draws.text_count != 2 || diagnostic.upload_bytes.image_byte_count != (pass ? 0U : 32U) ||
        (pass ? diagnostic.upload_bytes.glyph_atlas_byte_count != 0 : diagnostic.upload_bytes.glyph_atlas_byte_count == 0) ||
        stats.page_count != 1 || (pass ? stats.allocations_this_frame != 0 : stats.allocations_this_frame != 1) ||
        (pass ? stats.used_bytes != 0 : stats.used_bytes == 0))
      return dx12_fail(3, "immutable/duplicate/cached image and glyph uploads were counted or pooled incorrectly");
  }
  return 0;
}
