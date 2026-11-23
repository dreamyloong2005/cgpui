#include "dx12_test_support.hpp"
#include <limits>

using namespace cgpui;
using namespace cgpui_test;

namespace {
int rejected(RenderFrame& frame) {
  const auto result = frame.present();
  return !result && result.error().code == ErrorCode::invalid_argument
      ? 0 : dx12_fail(10, "invalid texture data or draw was not rejected");
}

int healthy(Renderer& renderer, const ImageAsset& image) {
  auto frame = renderer.begin_frame();
  if (!frame) return dx12_fail(11, "invalid image command poisoned renderer");
  (*frame)->upload_image(image);
  (*frame)->draw_image({.bounds = {{0, 0}, {16, 16}}, .asset = describe_image_asset(image)});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(12, "recovery capture failed");
  if (auto result = (*frame)->present(); !result) return dx12_fail(13, result.error().message.c_str());
  return dx12_pixel(renderer.last_frame_pixels(), 8, 8, {255, 0, 0, 255})
      ? 0 : dx12_fail(14, "valid upload after rejection rendered incorrectly");
}
} // namespace

int main() {
  Dx12Window window;
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(1, renderer.error().message.c_str());
  const ImageAsset valid{.id = {90}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 0, 0, 255}}};
  for (unsigned invalid_case = 0; invalid_case < 8; ++invalid_case) {
    auto invalid = valid;
    switch (invalid_case) {
      case 0: invalid.id.value = 0; break;
      case 1: invalid.bitmap.width = 0; break;
      case 2: invalid.bitmap.height = 0; break;
      case 3: invalid.bitmap.stride = 3; break;
      case 4: invalid.bitmap.pixels.resize(3); break;
      case 5: invalid.bitmap.format = static_cast<ImageFormat>(99); break;
      case 6: invalid.bitmap.width = (std::numeric_limits<std::uint32_t>::max)(); break;
      case 7: invalid.bitmap.height = (std::numeric_limits<std::uint32_t>::max)(); break;
    }
    auto frame = (*renderer)->begin_frame();
    if (!frame) return dx12_fail(2, "invalid upload begin failed");
    (*frame)->upload_image(invalid);
    if (const int result = rejected(**frame)) return result;
    if (const int result = healthy(**renderer, valid)) return result;
  }
  for (unsigned invalid_case = 0; invalid_case < 5; ++invalid_case) {
    ImageDraw invalid{.bounds = {{0, 0}, {16, 16}}, .asset = describe_image_asset(valid)};
    const float nan = std::numeric_limits<float>::quiet_NaN();
    switch (invalid_case) {
      case 0: invalid.bounds.origin.x = nan; break;
      case 1: invalid.source_rect = Rect{{nan, 0}, {1, 1}}; break;
      case 2: invalid.tint = Color{nan, 1, 1, 1}; break;
      case 3: invalid.clip_rect = Rect{{nan, 0}, {1, 1}}; break;
      case 4: invalid.sampling = static_cast<ImageSamplingMode>(99); break;
    }
    auto frame = (*renderer)->begin_frame();
    if (!frame) return dx12_fail(3, "invalid draw begin failed");
    (*frame)->draw_image(invalid);
    if (const int result = rejected(**frame)) return result;
    if (const int result = healthy(**renderer, valid)) return result;
  }
  return 0;
}
