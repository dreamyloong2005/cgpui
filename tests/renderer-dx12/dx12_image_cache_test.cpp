#include "dx12_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
ImageAsset image(std::array<std::uint8_t, 4> color) {
  return {.id = {42}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4,
          .pixels = {color[0], color[1], color[2], color[3]}}};
}

int expect_cached(Renderer& renderer, const ImageAssetDescriptor& asset,
                   std::array<std::uint8_t, 4> expected) {
  auto frame = renderer.begin_frame();
  if (!frame) return dx12_fail(10, frame.error().message.c_str());
  (*frame)->clear({0, 0, 0, 1});
  (*frame)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = asset});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(11, "cache capture failed");
  if (auto result = (*frame)->present(); !result) return dx12_fail(12, result.error().message.c_str());
  return dx12_pixel(renderer.last_frame_pixels(), 8, 8, expected)
      ? 0 : dx12_fail(13, "cached snapshot pixels differ");
}

int reject_frame(RenderFrame& frame) {
  const auto result = frame.present();
  return !result && result.error().code == ErrorCode::invalid_argument
      ? 0 : dx12_fail(14, "invalid image command was not rejected");
}
} // namespace

int main() {
  Dx12Window window;
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(1, renderer.error().message.c_str());
  auto red = image({255, 0, 0, 255});
  const auto green = image({0, 255, 0, 255});
  const auto blue = image({0, 0, 255, 255});
  const auto asset = describe_image_asset(red);
  {
    auto frame = (*renderer)->begin_frame();
    if (!frame) return dx12_fail(2, "snapshot begin failed");
    (*frame)->clear({0, 0, 0, 1});
    (*frame)->upload_image(red);
    (*frame)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = asset});
    // A caller-owned bitmap and a later same-id upload cannot rewrite this draw.
    red.bitmap.pixels[0] = 0;
    (*frame)->draw_rect({.rect = {{8, 8}, {16, 16}}, .color = {0, 0, 1, 1}});
    (*frame)->upload_image(green);
    (*frame)->draw_image({.bounds = {{16, 0}, {16, 32}}, .asset = asset});
    (*frame)->invalidate_image(asset.id);
    if (!(*frame)->request_pixel_capture()) return dx12_fail(3, "snapshot capture failed");
    if (auto result = (*frame)->present(); !result) return dx12_fail(4, result.error().message.c_str());
    const auto* pixels = (*renderer)->last_frame_pixels();
    if (!dx12_pixel(pixels, 4, 4, {255, 0, 0, 255}) ||
        !dx12_pixel(pixels, 12, 12, {0, 0, 255, 255}) ||
        !dx12_pixel(pixels, 20, 12, {0, 255, 0, 255}))
      return dx12_fail(5, "same-frame image replacement/invalidation or primitive order differs");
  }
  {
    auto frame = (*renderer)->begin_frame();
    if (!frame) return dx12_fail(6, "invalidation begin failed");
    (*frame)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = asset});
    if (const int result = reject_frame(**frame)) return result;
  }
  {
    auto abandoned = (*renderer)->begin_frame();
    if (!abandoned) return dx12_fail(7, "abandoned begin failed");
    // CPU snapshots are record-time cache updates, not a public transaction.
    // Abandoning must not mark an unsubmitted GPU texture as uploaded.
    (*abandoned)->upload_image(blue);
    (*abandoned)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = asset});
  }
  if (const int result = expect_cached(**renderer, asset, {0, 0, 255, 255})) return result;
  {
    auto failed = (*renderer)->begin_frame();
    if (!failed) return dx12_fail(8, "failed-frame begin failed");
    (*failed)->upload_image(green);
    auto missing = asset;
    missing.id.value = 999;
    (*failed)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = missing});
    if (const int result = reject_frame(**failed)) return result;
  }
  if (const int result = expect_cached(**renderer, asset, {0, 255, 0, 255})) return result;
  {
    auto failed = (*renderer)->begin_frame();
    if (!failed) return dx12_fail(9, "invalid upload begin failed");
    auto invalid = blue;
    invalid.bitmap.stride = 0;
    (*failed)->upload_image(invalid);
    if (const int result = reject_frame(**failed)) return result;
  }
  if (const int result = expect_cached(**renderer, asset, {0, 255, 0, 255})) return result;
  {
    auto failed = (*renderer)->begin_frame();
    if (!failed) return dx12_fail(15, "descriptor mismatch begin failed");
    auto invalid = asset;
    invalid.pixel_width = 2;
    (*failed)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = invalid});
    if (const int result = reject_frame(**failed)) return result;
  }
  return expect_cached(**renderer, asset, {0, 255, 0, 255});
}
