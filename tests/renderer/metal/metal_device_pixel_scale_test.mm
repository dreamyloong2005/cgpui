#include "metal_pixel_test_support.hpp"

// At DpiScale 2 a 64 x 64 drawable is 32 x 32 logical pixels: rects, radii,
// clips, transform translations and image bounds are drawn at twice their
// logical size (renderer_to_device_pixels), as on Vulkan.
int main() {
  @autoreleasepool {
    const cgpui::ImageAsset image{
        .id = {7101},
        .logical_size = {1.0F, 1.0F},
        .bitmap = {.width = 1, .height = 1, .stride = 4,
                   .pixels = {255, 255, 255, 255}}};
    cgpui_test::MetalPixelTestSurface surface;
    auto pixels = cgpui_test::capture_metal_pixels(
        surface,
        cgpui::Color{.a = 1.0F},
        [&](cgpui::RenderFrame& frame) {
          frame.draw_rect(cgpui::SolidRect{
              .rect = {.origin = {2.0F, 2.0F}, .size = {12.0F, 12.0F}},
              .color = {.r = 1.0F, .a = 1.0F}});
          frame.draw_rounded_rect(cgpui::RoundedRectDraw{
              .rect = {.origin = {18.0F, 2.0F}, .size = {12.0F, 12.0F}},
              .color = {.g = 1.0F, .a = 1.0F},
              .radius = cgpui::BorderRadii::all(6.0F)});
          frame.draw_rect(cgpui::SolidRect{
              .rect = {.origin = {2.0F, 18.0F}, .size = {12.0F, 12.0F}},
              .color = {.b = 1.0F, .a = 1.0F},
              .clip_rect = cgpui::Rect{{2.0F, 18.0F}, {6.0F, 12.0F}}});
          frame.draw_rect(cgpui::SolidRect{
              .rect = {.size = {8.0F, 8.0F}},
              .color = {.r = 1.0F, .g = 1.0F, .a = 1.0F},
              .metadata = {.transform = cgpui::AffineTransform{
                               .translate_x = 20.0F, .translate_y = 20.0F}}});
          frame.upload_image(image);
          frame.draw_image(cgpui::ImageDraw{
              .bounds = {.origin = {28.0F, 16.0F}, .size = {4.0F, 2.0F}},
              .asset = cgpui::describe_image_asset(image),
              .tint = cgpui::Color{.r = 1.0F, .b = 1.0F, .a = 1.0F},
              .sampling = cgpui::ImageSamplingMode::nearest});
        },
        {},
        cgpui::DpiScale{2.0F});
    if (!pixels || !pixels->valid() || pixels->width != 64) return 1;
    using cgpui_test::metal_pixel_near;
    const std::array<std::uint8_t, 4> black{0, 0, 0, 255};
    // Solid rect: logical 2..14 is device 4..28.
    if (!metal_pixel_near(*pixels, 24, 24, {255, 0, 0, 255}) ||
        !metal_pixel_near(*pixels, 2, 2, black)) return 2;
    // Rounded rect: device 36..60 x 4..28 with a 12 px device radius.
    if (!metal_pixel_near(*pixels, 48, 16, {0, 255, 0, 255}) ||
        !metal_pixel_near(*pixels, 37, 5, black)) return 3;
    // Clip rect: logical x 2..8 is device 4..16; the rect beyond is clipped.
    if (!metal_pixel_near(*pixels, 10, 48, {0, 0, 255, 255}) ||
        !metal_pixel_near(*pixels, 20, 48, black)) return 4;
    // Transform translation: logical 20..28 is device 40..56.
    if (!metal_pixel_near(*pixels, 52, 52, {255, 255, 0, 255}) ||
        !metal_pixel_near(*pixels, 38, 38, black)) return 5;
    // Image bounds: logical 28..32 x 16..18 is device 56..64 x 32..36.
    if (!metal_pixel_near(*pixels, 60, 34, {255, 0, 255, 255})) return 6;
    return 0;
  }
}
