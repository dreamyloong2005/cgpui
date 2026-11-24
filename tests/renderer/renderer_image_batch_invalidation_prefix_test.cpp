#include "renderer_pixel_test_support.hpp"
#include "../support/frame_wait_test_support.hpp"
#include <iostream>

using namespace cgpui;
using namespace cgpui_test;

namespace {
int prefix_case(RendererPixelTestWindow& window, bool invalidate_before_upload) {
  auto renderer = create_pixel_test_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  if (!renderer) return 2;
  const ImageAsset green{.id = {9951}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {0, 255, 0, 255}}};
  const auto asset = describe_image_asset(green);
  {
    auto frame = wait_for_test_frame(**renderer);
    if (!frame) { report_pixel_test_error("prefix initial begin", frame.error()); return 3; }
    (*frame)->clear({0, 0, 0, 1});
    if (invalidate_before_upload) (*frame)->invalidate_image(asset.id);
    (*frame)->upload_image(green);
    (*frame)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = asset,
        .sampling = ImageSamplingMode::nearest});
    if (!invalidate_before_upload) {
      (*frame)->invalidate_image(asset.id);
      (*frame)->invalidate_image(asset.id); // Duplicate IDs still form one prefix, not a late erase.
    }
    if (!pixel_test_ok((*frame)->request_pixel_capture(), "prefix initial capture")) return 4;
    if (!pixel_test_ok((*frame)->present(), "prefix initial native present")) return 5;
    const auto* pixels = (*renderer)->last_frame_pixels();
    if (!pixels || !pixels->valid() || !pixel_near(*pixels, 8, 8, {0, 255, 0, 255}) ||
        !pixel_near(*pixels, 48, 48, {0, 0, 0, 255})) return 6;
  }
  std::cout << "IMAGE_BATCH_PREFIX initial_native_pixels=success before=" << invalidate_before_upload << '\n';
  auto cached = wait_for_test_frame(**renderer);
  if (!cached) { report_pixel_test_error("prefix cached begin", cached.error()); return 7; }
  (*cached)->clear({1, 0, 0, 1}); // New background distinguishes a fresh capture from previous green/black history.
  (*cached)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = asset,
      .sampling = ImageSamplingMode::nearest});
  if (!pixel_test_ok((*cached)->request_pixel_capture(), "prefix cached capture")) return 8;
  const auto presented = (*cached)->present(); // Valid previously supplied asset, not an intentionally missing ID.
  if (!presented) {
    report_pixel_test_error("prefix cached present", presented.error());
    if (presented.error().code != ErrorCode::invalid_argument) return 9;
    std::cerr << "FAIL_IMAGE_INVALIDATION_PREFIX: supplied upload was erased after its frame prefix\n";
    return 40;
  }
  const auto* pixels = (*renderer)->last_frame_pixels();
  if (!pixels || !pixels->valid()) return 10;
  if (!pixel_near(*pixels, 8, 8, {0, 255, 0, 255}) ||
      !pixel_near(*pixels, 48, 48, {255, 0, 0, 255})) {
    std::cerr << "FAIL_IMAGE_INVALIDATION_PREFIX: final uploaded cache pixels or fresh capture differ\n";
    return 40;
  }
  return 0;
}
} // namespace

int main() {
  RendererPixelTestWindow window(L"CGPUIImageBatchPrefix", L"Image invalidation prefix contract");
  if (!window.valid()) return 1;
  if (const int control = prefix_case(window, true)) return control;
  return prefix_case(window, false);
}
