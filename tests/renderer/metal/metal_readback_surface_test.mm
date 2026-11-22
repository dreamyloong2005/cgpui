#include "metal_pixel_test_support.hpp"

#import <QuartzCore/CAMetalLayer.h>

// Capture is requested after begin_frame(), so the acquired drawable must
// already permit readback; flipping framebufferOnly on present is too late.
int main() {
  @autoreleasepool {
    cgpui_test::MetalPixelTestSurface surface;
    if (!surface.valid()) return 1;
    auto renderer = cgpui::create_renderer({
        .native_surface = surface.surface(), .framebuffer_size = {64, 64}, .scale = {1}});
    if (!renderer) return 2;
    CAMetalLayer* layer = (__bridge CAMetalLayer*)
        std::get<cgpui::MetalSurfaceHandle>(surface.surface()).layer;
    for (bool transparent : {false, true}) {
      if (!(*renderer)->set_transparent_surface(transparent) ||
          !(*renderer)->resize({64, 64}, {1})) return 3;
      if (layer.framebufferOnly) return 4;
      auto frame = (*renderer)->begin_frame();
      if (!frame || !(*frame)->request_pixel_capture()) return 5;
      (*frame)->clear({.r = 0.25F, .g = 0.5F, .b = 0.75F, .a = 1.0F});
      if (!(*frame)->present()) return 6;
      const auto* pixels = (*renderer)->last_frame_pixels();
      if (!pixels || !cgpui_test::metal_pixel_near(*pixels, 32, 32, {64, 128, 191, 255})) {
        return 7;
      }
    }
  }
  return 0;
}
