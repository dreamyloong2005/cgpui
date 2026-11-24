#include "../support/frame_wait_test_support.hpp"
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
  auto frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) return dx12_fail(10, frame.error().message.c_str());
  (*frame)->clear({0, 0, 0, 1});
  (*frame)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = asset});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(11, "cache capture failed");
  if (auto result = (*frame)->present(); !result) return dx12_fail(12, result.error().message.c_str());
  return dx12_pixel(renderer.last_frame_pixels(), 8, 8, expected)
      ? 0 : dx12_fail(13, "cached snapshot pixels differ");
}

int expect_missing(Renderer& renderer, RenderFrame& frame) {
  frame.clear({0, 1, 1, 1});
  frame.draw_rect({.rect = {{0, 0}, {8, 8}}, .color = {0, 0, 1, 1}});
  if (!frame.request_pixel_capture() || !frame.present()) return dx12_fail(20, "missing image must continue frame");
  const auto* s = renderer.last_frame_diagnostic_snapshot();
  return dx12_pixel(renderer.last_frame_pixels(), 4, 4, {0, 0, 255, 255}) &&
      dx12_pixel(renderer.last_frame_pixels(), 16, 16, {0, 255, 255, 255}) && s &&
      s->planned_draws.image_count == 1 && s->submitted_draws.image_count == 0 &&
      s->dropped_resources.missing_submission_resource_count == 1 && !s->work.exact_match()
      ? 0 : dx12_fail(21, "missing image has stale pixels or false submission");
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
    auto frame = cgpui_test::wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(2, "snapshot begin failed");
    (*frame)->clear({0, 0, 0, 1});
    (*frame)->upload_image(red);
    (*frame)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = asset});
    // Caller bytes are copied, but all draws resolve the final same-ID upload.
    red.bitmap.pixels[0] = 0;
    (*frame)->draw_rect({.rect = {{8, 8}, {16, 16}}, .color = {0, 0, 1, 1}});
    (*frame)->upload_image(green);
    (*frame)->draw_image({.bounds = {{16, 0}, {16, 32}}, .asset = asset});
    (*frame)->invalidate_image(asset.id);
    if (!(*frame)->request_pixel_capture()) return dx12_fail(3, "snapshot capture failed");
    if (auto result = (*frame)->present(); !result) return dx12_fail(4, result.error().message.c_str());
    const auto* pixels = (*renderer)->last_frame_pixels();
    if (!dx12_pixel(pixels, 4, 4, {0, 255, 0, 255}) ||
        !dx12_pixel(pixels, 12, 12, {0, 0, 255, 255}) ||
        !dx12_pixel(pixels, 20, 12, {0, 255, 0, 255}))
      return dx12_fail(5, "same-frame image replacement/invalidation or primitive order differs");
  }
  if (const int result = expect_cached(**renderer, asset, {0, 255, 0, 255})) return result;
  {
    auto invalidation = cgpui_test::wait_for_test_frame(**renderer);
    if (!invalidation) return dx12_fail(16, "invalidation-only begin failed");
    (*invalidation)->invalidate_image(asset.id);
    if (!(*invalidation)->present()) return dx12_fail(17, "invalidation-only Present failed");
  }
  {
    auto frame = cgpui_test::wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(6, "invalidation begin failed");
    (*frame)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = asset});
    if (const int result = expect_missing(**renderer, **frame)) return result;
  }
  {
    auto seed = cgpui_test::wait_for_test_frame(**renderer);
    if (!seed) return dx12_fail(18, "known-cache seed unavailable");
    (*seed)->upload_image(green);
    if (!(*seed)->present()) return dx12_fail(19, "known-cache seed failed");
  }
  {
    auto abandoned = cgpui_test::wait_for_test_frame(**renderer);
    if (!abandoned) return dx12_fail(7, "abandoned begin failed");
    // Pending replacement is frame-owned; abandonment never applies this prefix.
    (*abandoned)->upload_image(blue);
    (*abandoned)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = asset});
  }
  if (const int result = expect_cached(**renderer, asset, {0, 255, 0, 255})) return result;
  {
    auto failed = cgpui_test::wait_for_test_frame(**renderer);
    if (!failed) return dx12_fail(8, "failed-frame begin failed");
    (*failed)->upload_image(green);
    // Valid absence is a successful omission, not a rollback test.
    auto missing = asset;
    missing.id.value = 999;
    (*failed)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = missing});
    if (const int result = expect_missing(**renderer, **failed)) return result;
  }
  if (const int result = expect_cached(**renderer, asset, {0, 255, 0, 255})) return result;
  {
    auto failed = cgpui_test::wait_for_test_frame(**renderer);
    if (!failed) return dx12_fail(9, "invalid upload begin failed");
    auto invalid = blue;
    invalid.bitmap.stride = 0;
    (*failed)->upload_image(invalid);
    if (const int result = reject_frame(**failed)) return result;
  }
  if (const int result = expect_cached(**renderer, asset, {0, 255, 0, 255})) return result;
  {
    auto failed = cgpui_test::wait_for_test_frame(**renderer);
    if (!failed) return dx12_fail(15, "descriptor mismatch begin failed");
    auto invalid = asset;
    invalid.pixel_width = 2;
    (*failed)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = invalid});
    if (const int result = reject_frame(**failed)) return result;
  }
  return expect_cached(**renderer, asset, {0, 255, 0, 255});
}
