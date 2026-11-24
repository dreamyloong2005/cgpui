#include "renderer_pixel_test_support.hpp"
#include "../support/frame_wait_test_support.hpp"
#include <iostream>

using namespace cgpui;
using namespace cgpui_test;

namespace {
int abandoned_case(RendererPixelTestWindow& window, bool invalidate) {
  auto renderer = create_pixel_test_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  if (!renderer) return 2;
  const ImageAsset red{.id = {9952}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 0, 0, 255}}};
  auto blue = red;
  blue.bitmap.pixels = {0, 0, 255, 255};
  const auto asset = describe_image_asset(red);
  {
    auto seed = wait_for_test_frame(**renderer);
    if (!seed) { report_pixel_test_error("abandonment seed begin", seed.error()); return 3; }
    (*seed)->clear({0, 0, 0, 1});
    (*seed)->upload_image(red);
    (*seed)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = asset,
        .sampling = ImageSamplingMode::nearest});
    if (!pixel_test_ok((*seed)->request_pixel_capture(), "abandonment seed capture")) return 4;
    if (!pixel_test_ok((*seed)->present(), "abandonment seed native present")) return 5;
    const auto* pixels = (*renderer)->last_frame_pixels();
    if (!pixels || !pixels->valid() || !pixel_near(*pixels, 8, 8, {255, 0, 0, 255}) ||
        !pixel_near(*pixels, 48, 48, {0, 0, 0, 255})) return 6;
  }
  std::cout << "IMAGE_BATCH_ABANDONMENT seeded_native_pixels=red invalidate=" << invalidate << '\n';
  {
    auto abandoned = wait_for_test_frame(**renderer);
    if (!abandoned) { report_pixel_test_error("abandoned begin", abandoned.error()); return 7; }
    if (invalidate) (*abandoned)->invalidate_image(asset.id);
    else (*abandoned)->upload_image(blue);
    // No Present: this separately tests abandonment of replacement and invalidation,
    // not rollback after arbitrary failed submission or behavior of an unprovided ID.
  }
  auto cached = wait_for_test_frame(**renderer);
  if (!cached) { report_pixel_test_error("post-abandon begin", cached.error()); return 8; }
  (*cached)->clear({0, 1, 0, 1}); // A stale red/black capture must not satisfy this fresh red/green frame.
  (*cached)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = asset,
      .sampling = ImageSamplingMode::nearest});
  if (!pixel_test_ok((*cached)->request_pixel_capture(), "post-abandon capture")) return 9;
  const auto presented = (*cached)->present();
  if (!presented) {
    report_pixel_test_error("post-abandon present", presented.error());
    if (presented.error().code != ErrorCode::invalid_argument) return 10;
    std::cerr << "FAIL_IMAGE_BATCH_ABANDONMENT: abandoned invalidation removed the successfully seeded asset\n";
    return 40;
  }
  const auto* pixels = (*renderer)->last_frame_pixels();
  if (!pixels || !pixels->valid()) return 11;
  if (!pixel_near(*pixels, 8, 8, {255, 0, 0, 255}) ||
      !pixel_near(*pixels, 48, 48, {0, 255, 0, 255})) {
    std::cerr << "FAIL_IMAGE_BATCH_ABANDONMENT: abandoned replacement changed cached pixels or capture is stale\n";
    return 40;
  }
  return 0;
}
} // namespace

int main() {
  RendererPixelTestWindow window(L"CGPUIImageBatchAbandonment", L"Image batch abandonment contract");
  if (!window.valid()) return 1;
  const int replacement = abandoned_case(window, false);
  const int invalidation = abandoned_case(window, true); // Execute both independent cases even if replacement differs.
  return replacement ? replacement : invalidation;
}
