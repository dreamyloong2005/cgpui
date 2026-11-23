#include "dx12_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "image test native window failed");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  auto frame = (*renderer)->begin_frame();
  if (!frame) return dx12_fail(3, frame.error().message.c_str());
  const ImageAsset image{.id = {41}, .logical_size = {2, 2},
      .bitmap = {.width = 2, .height = 2, .stride = 8,
          .pixels = {255, 0, 0, 255, 0, 255, 0, 255,
                     0, 0, 255, 255, 255, 255, 255, 255}}};
  (*frame)->clear({0, 0, 0, 1});
  (*frame)->upload_image(image);
  (*frame)->draw_image({.bounds = {{8, 8}, {32, 32}},
      .asset = describe_image_asset(image), .sampling = ImageSamplingMode::nearest});
  auto requested = (*frame)->request_pixel_capture();
  if (!requested) return dx12_fail(4, requested.error().message.c_str());
  auto presented = (*frame)->present();
  if (!presented) return dx12_fail(5, presented.error().message.c_str());
  const auto* pixels = (*renderer)->last_frame_pixels();
  if (!dx12_pixel(pixels, 16, 16, {255, 0, 0, 255}) ||
      !dx12_pixel(pixels, 32, 16, {0, 255, 0, 255}) ||
      !dx12_pixel(pixels, 16, 32, {0, 0, 255, 255}) ||
      !dx12_pixel(pixels, 32, 32, {255, 255, 255, 255}) ||
      !dx12_pixel(pixels, 4, 4, {0, 0, 0, 255})) {
    return dx12_fail(6, "nearest image pixels differ");
  }
  return 0;
}
