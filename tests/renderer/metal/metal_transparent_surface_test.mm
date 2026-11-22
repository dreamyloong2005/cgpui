#include "metal_pixel_test_support.hpp"

// A transparent CAMetalLayer composites premultiplied alpha: the clear
// stays alpha 0 and a half-transparent draw is stored premultiplied.
int main() {
  @autoreleasepool {
    cgpui_test::MetalPixelTestSurface surface;
    if (!surface.valid()) return 1;
    auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
        .native_surface = surface.surface(),
        .framebuffer_size = {64.0F, 64.0F},
        .scale = {1.0F}});
    if (!renderer) return 2;
    if (!(*renderer)->popup_windows_supported()) return 9;
    const cgpui::RendererSurfaceComposition opaque =
        (*renderer)->surface_composition();
    if (!opaque.supported || opaque.transparent_requested ||
        opaque.composite_alpha != cgpui::RendererCompositeAlpha::opaque ||
        opaque.alpha_composited()) {
      return 3;
    }
    auto transparent = (*renderer)->set_transparent_surface(true);
    if (!transparent || !transparent->transparent_requested ||
        transparent->composite_alpha !=
            cgpui::RendererCompositeAlpha::pre_multiplied ||
        !transparent->alpha_composited() ||
        !(*renderer)->surface_composition().alpha_composited()) {
      return 4;
    }

    auto frame = (*renderer)->begin_frame();
    if (!frame || !(*frame)->request_pixel_capture()) return 5;
    (*frame)->clear(cgpui::Color{.a = 0.0F});
    (*frame)->draw_rect(cgpui::SolidRect{
        .rect = {.origin = {16.0F, 16.0F}, .size = {32.0F, 32.0F}},
        .color = {.r = 1.0F, .a = 0.5F}});
    if (!(*frame)->present()) return 6;
    const cgpui::RendererFramePixels* pixels = (*renderer)->last_frame_pixels();
    if (pixels == nullptr ||
        !cgpui_test::metal_pixel_near(*pixels, 4, 4, {0, 0, 0, 0}) ||
        !cgpui_test::metal_pixel_near(*pixels, 32, 32, {128, 0, 0, 128})) {
      return 7;
    }
    (*frame).reset();

    auto restored = (*renderer)->set_transparent_surface(false);
    return restored && !restored->transparent_requested &&
                   restored->composite_alpha ==
                       cgpui::RendererCompositeAlpha::opaque
               ? 0
               : 8;
  }
}
