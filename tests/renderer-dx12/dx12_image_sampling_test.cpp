#include "dx12_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
int sample_filters(Renderer& renderer) {
  // Padded input rows exercise repacking before DX12's aligned upload footprint.
  const ImageAsset colors{.id = {80}, .logical_size = {2, 2},
      .bitmap = {.width = 2, .height = 2, .stride = 12,
          .pixels = {255, 0, 0, 255, 0, 255, 0, 255, 13, 14, 15, 16,
                     0, 0, 255, 255, 255, 255, 255, 255, 23, 24, 25, 26}}};
  auto frame = renderer.begin_frame();
  if (!frame) return dx12_fail(10, "sampling begin failed");
  (*frame)->clear({0, 0, 0, 1});
  (*frame)->upload_image(colors);
  (*frame)->draw_image({.bounds = {{0, 0}, {9, 9}}, .asset = describe_image_asset(colors),
      .sampling = ImageSamplingMode::nearest});
  (*frame)->draw_image({.bounds = {{16, 0}, {9, 9}}, .asset = describe_image_asset(colors),
      .sampling = ImageSamplingMode::linear});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(11, "sampling capture failed");
  if (auto result = (*frame)->present(); !result) return dx12_fail(12, result.error().message.c_str());
  const auto* pixels = renderer.last_frame_pixels();
  // Odd-sized quad puts its central pixel exactly at the four-texel junction.
  return dx12_pixel(pixels, 4, 4, {255, 255, 255, 255}) &&
      dx12_pixel(pixels, 20, 4, {128, 128, 128, 255}) &&
      dx12_pixel(pixels, 16, 0, {255, 0, 0, 255}) &&
      dx12_pixel(pixels, 24, 8, {255, 255, 255, 255})
      ? 0 : dx12_fail(13, "nearest/linear/clamp or padded upload differs");
}

int crop_transform_clip(Renderer& renderer, float scale) {
  // source_rect remains in texels while geometry, translations and clips scale.
  const ImageAsset colors{.id = {81}, .logical_size = {2, 2},
      .bitmap = {.width = 2, .height = 2, .stride = 8,
          .pixels = {255, 0, 0, 255, 0, 255, 0, 255,
                     0, 0, 255, 255, 255, 255, 255, 255}}};
  const PaintMetadata ancestry[] = {{.opacity = 0.75F},
      {.opacity = 0.5F, .transform = AffineTransform::translation(2, 3)}};
  const Rect clips[] = {{{6, 7}, {6, 6}}, {{7, 8}, {6, 6}}};
  auto frame = renderer.begin_frame();
  if (!frame) return dx12_fail(20, "crop begin failed");
  (*frame)->clear({0, 0, 1, 1});
  (*frame)->upload_image(colors);
  (*frame)->draw_image({.bounds = {{2, 2}, {12, 12}}, .asset = describe_image_asset(colors),
      .source_rect = Rect{{1, 1}, {1, 1}}, .tint = Color{0.5F, 1, 0, 0.5F},
      .sampling = ImageSamplingMode::nearest, .clip_rect = Rect{{6, 7}, {4, 4}},
      .clip_stack = renderer_clip_stack_record(clips),
      .composition_stack = renderer_composition_stack_record(ancestry), .metadata = ancestry[1]});
  // Image→rect transition must restore the rectangle root/PSO; rect→image is
  // covered by the cache test. This rectangle is outside the effective clip.
  (*frame)->draw_rect({.rect = {{20, 20}, {4, 4}}, .color = {1, 0, 0, 1}});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(21, "crop capture failed");
  if (auto result = (*frame)->present(); !result) return dx12_fail(22, result.error().message.c_str());
  const auto* pixels = renderer.last_frame_pixels();
  // white texel * tint, alpha .5 * cumulative opacity .5 over blue:
  // rgb=(.125,.25,.75), independently of DPI and ancestor count.
  return dx12_pixel(pixels, static_cast<unsigned>(8 * scale), static_cast<unsigned>(9 * scale),
                       {32, 64, 191, 255}) &&
      dx12_pixel(pixels, static_cast<unsigned>(6 * scale), static_cast<unsigned>(9 * scale), {0, 0, 255, 255}) &&
      dx12_pixel(pixels, static_cast<unsigned>(10 * scale), static_cast<unsigned>(9 * scale), {0, 0, 255, 255}) &&
      dx12_pixel(pixels, static_cast<unsigned>(8 * scale), static_cast<unsigned>(7 * scale), {0, 0, 255, 255}) &&
      dx12_pixel(pixels, static_cast<unsigned>(21 * scale), static_cast<unsigned>(21 * scale), {255, 0, 0, 255})
      ? 0 : dx12_fail(23, "crop/tint/opacity/transform/clip/DPI differs");
}
} // namespace

int main() {
  Dx12Window window;
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(1, renderer.error().message.c_str());
  if (const int result = sample_filters(**renderer)) return result;
  for (float scale : {1.0F, 2.0F}) {
    if (!(*renderer)->resize({64, 64}, {scale})) return dx12_fail(2, "sampling DPI resize failed");
    if (const int result = crop_transform_clip(**renderer, scale)) return result;
  }
  return 0;
}
