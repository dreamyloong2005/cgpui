#include "metal_pixel_test_support.hpp"

#include <cstdio>

// Backdrop blur on Metal: the region under the backdrop is blurred, pixels
// outside it are untouched, the tint mixes over the blur, draws after the
// backdrop stay sharp, and frames without a backdrop do no offscreen work.
namespace {

constexpr cgpui::Color black{.a = 1.0F};
constexpr cgpui::Color white{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F};

// 2 px black and white columns over the whole 64 x 64 frame.
void stripes(cgpui::RenderFrame& frame) {
  for (float x = 0.0F; x < 64.0F; x += 4.0F) {
    frame.draw_rect(cgpui::SolidRect{.rect = {{x, 0.0F}, {2.0F, 64.0F}}, .color = white});
  }
}

bool channel_between(const cgpui::RendererFramePixels& pixels, std::uint32_t x,
                     std::uint32_t y, std::size_t channel, int low, int high) {
  const auto pixel = pixels.pixel_rgba8(x, y);
  const int value = pixel ? (*pixel)[channel] : -1;
  if (value < low || value > high) {
    std::fprintf(stderr, "pixel %u,%u channel %zu = %d, want %d..%d\n", x, y, channel,
                 value, low, high);
    return false;
  }
  return true;
}

}  // namespace

int main() {
  @autoreleasepool {
    cgpui_test::MetalPixelTestSurface surface;
    if (!surface.valid()) return 1;
    auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
        .native_surface = surface.surface(), .framebuffer_size = {64, 64}, .scale = {1}});
    if (!renderer || !(*renderer)->backdrop_blur_support().supported) return 2;

    auto frame = (*renderer)->begin_frame();
    if (!frame || !(*frame)->request_pixel_capture()) return 3;
    (*frame)->clear(black);
    stripes(**frame);
    (*frame)->draw_backdrop_blur(cgpui::BackdropBlurDraw{
        .rect = {{16, 16}, {32, 32}}, .blur_radius = 4.0F});
    (*frame)->draw_backdrop_blur(cgpui::BackdropBlurDraw{
        .rect = {{0, 52}, {64, 12}}, .blur_radius = 4.0F,
        .tint = cgpui::Color{.r = 1.0F, .a = 0.5F}});
    (*frame)->draw_rect(cgpui::SolidRect{.rect = {{30, 30}, {4, 4}},
                                         .color = {.b = 1.0F, .a = 1.0F}});
    if (!(*frame)->present()) return 4;
    const cgpui::RendererFramePixels* pixels = (*renderer)->last_frame_pixels();
    if (pixels == nullptr) return 5;
    // Outside: exact stripes. Inside: the 2 px stripes average to mid gray.
    // The tinted strip mixes red at 50 % over the blur. The rect drawn
    // after the backdrops is sharp.
    if (!cgpui_test::metal_pixel_near(*pixels, 4, 30, {255, 255, 255, 255}, 0) ||
        !cgpui_test::metal_pixel_near(*pixels, 6, 30, {0, 0, 0, 255}, 0) ||
        !channel_between(*pixels, 24, 24, 0, 107, 148) ||
        !channel_between(*pixels, 25, 24, 0, 107, 148) ||
        !channel_between(*pixels, 32, 58, 0, 170, 212) ||
        !channel_between(*pixels, 32, 58, 1, 43, 85) ||
        !cgpui_test::metal_pixel_near(*pixels, 31, 31, {0, 0, 255, 255}, 0)) {
      return 6;
    }
    const auto* snapshot = (*renderer)->last_frame_diagnostic_snapshot();
    if (snapshot == nullptr || snapshot->backdrop_blur.blurred_backdrop_count != 2 ||
        snapshot->backdrop_blur.offscreen_pass_count != 3 ||
        !snapshot->backdrop_blur.offscreen_targets_allocated) {
      return 7;
    }
    (*frame).reset();

    auto plain = (*renderer)->begin_frame();
    if (!plain || !(*plain)->request_pixel_capture()) return 8;
    (*plain)->clear(black);
    stripes(**plain);
    if (!(*plain)->present()) return 9;
    const auto* plain_snapshot = (*renderer)->last_frame_diagnostic_snapshot();
    const cgpui::RendererFramePixels* plain_pixels = (*renderer)->last_frame_pixels();
    return plain_snapshot != nullptr &&
                   plain_snapshot->backdrop_blur.blurred_backdrop_count == 0 &&
                   plain_snapshot->backdrop_blur.offscreen_pass_count == 0 &&
                   plain_pixels != nullptr &&
                   cgpui_test::metal_pixel_near(*plain_pixels, 24, 24, {255, 255, 255, 255}, 0)
               ? 0
               : 10;
  }
}
