#include "renderer_pixel_test_support.hpp"
#include "../support/frame_wait_test_support.hpp"
#include <iostream>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  RendererPixelTestWindow window(L"CGPUIImageBatchLastUpload", L"Image final upload contract");
  if (!window.valid()) return 1;
  auto renderer = create_pixel_test_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  if (!renderer) return 2; // Actual requested backend, with identity validation and no fallback.
  const ImageAsset red{.id = {9950}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 0, 0, 255}}};
  auto green = red;
  green.bitmap.pixels = {0, 255, 0, 255};
  const auto asset = describe_image_asset(red);
  auto frame = wait_for_test_frame(**renderer);
  if (!frame) { report_pixel_test_error("final upload begin", frame.error()); return 3; }
  (*frame)->clear({0, 0, 0, 1});
  (*frame)->upload_image(red);
  (*frame)->draw_image({.bounds = {{0, 0}, {32, 32}}, .asset = asset,
      .sampling = ImageSamplingMode::nearest});
  (*frame)->draw_rect({.rect = {{8, 8}, {16, 16}}, .color = {0, 0, 1, 1}});
  (*frame)->upload_image(green); // The final same-ID upload supplies BOTH image draw calls.
  green.bitmap.pixels = {255, 0, 0, 255}; // Caller mutation after upload cannot rewrite its owned copy.
  (*frame)->draw_image({.bounds = {{16, 0}, {16, 32}}, .asset = asset,
      .sampling = ImageSamplingMode::nearest});
  if (!pixel_test_ok((*frame)->request_pixel_capture(), "final upload capture")) return 4;
  if (!pixel_test_ok((*frame)->present(), "final upload native present")) return 5; // Never retried.
  const auto* pixels = (*renderer)->last_frame_pixels();
  if (!pixels || !pixels->valid() || pixels->width <= 48 || pixels->height <= 48) return 6;
  std::cout << "IMAGE_BATCH_FINAL_UPLOAD native_present=success capture=valid\n";
  if (!pixel_near(*pixels, 4, 4, {0, 255, 0, 255}) ||
      !pixel_near(*pixels, 12, 12, {0, 0, 255, 255}) ||
      !pixel_near(*pixels, 20, 12, {0, 255, 0, 255}) ||
      !pixel_near(*pixels, 48, 48, {0, 0, 0, 255})) {
    std::cerr << "FAIL_IMAGE_FINAL_UPLOAD: final same-ID bytes/caller copy/mixed draw order differ\n";
    return 40;
  }
  return 0;
}
