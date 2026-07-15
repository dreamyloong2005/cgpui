#include "metal_pixel_test_support.hpp"

int main() {
  @autoreleasepool {
    cgpui_test::MetalPixelTestSurface surface;
    auto pixels = cgpui_test::capture_metal_pixels(
        surface,
        cgpui::Color{.a = 1.0F},
        [](cgpui::RenderFrame& frame) {
          frame.draw_rect(cgpui::SolidRect{
              .rect = {.origin = {8.0F, 8.0F}, .size = {20.0F, 20.0F}},
              .color = {.r = 1.0F, .a = 1.0F}});
          frame.draw_rounded_rect(cgpui::RoundedRectDraw{
              .rect = {.origin = {36.0F, 8.0F}, .size = {20.0F, 20.0F}},
              .color = {.g = 1.0F, .a = 1.0F},
              .radius = {.top_left = 8.0F,
                         .top_right = 8.0F,
                         .bottom_right = 8.0F,
                         .bottom_left = 8.0F}});
          frame.draw_rounded_rect(cgpui::RoundedRectDraw{
              .rect = {.origin = {36.0F, 36.0F}, .size = {20.0F, 20.0F}},
              .color = {.r = 1.0F, .a = 1.0F},
              .radius = cgpui::BorderRadii::all(6.0F),
              .fill_enabled = false,
              .border_color = cgpui::Color{.g = 1.0F, .b = 1.0F, .a = 1.0F},
              .border_width = 3.0F});
          frame.draw_text_selection(cgpui::TextSelectionDraw{
              .rect = {.origin = {8.0F, 40.0F}, .size = {20.0F, 12.0F}},
              .color = {.b = 1.0F, .a = 1.0F}});
          frame.draw_text_caret(cgpui::TextCaretDraw{
              .rect = {.origin = {40.0F, 40.0F}, .size = {2.0F, 16.0F}},
              .color = {.r = 1.0F, .g = 1.0F, .a = 1.0F}});
        });
    return pixels && pixels->valid() && pixels->width == 64 &&
                   pixels->height == 64 && pixels->rgba8.size() == 64 * 64 * 4 &&
                   cgpui_test::metal_pixel_near(*pixels, 16, 16, {255, 0, 0, 255}) &&
                   cgpui_test::metal_pixel_near(*pixels, 46, 18, {0, 255, 0, 255}) &&
                   cgpui_test::metal_pixel_near(*pixels, 36, 8, {0, 0, 0, 255}) &&
                   cgpui_test::metal_pixel_near(*pixels, 16, 46, {0, 0, 255, 255}) &&
                   cgpui_test::metal_pixel_near(*pixels, 40, 48, {255, 255, 0, 255}) &&
                   cgpui_test::metal_pixel_near(*pixels, 46, 38, {0, 255, 255, 255}) &&
                   cgpui_test::metal_pixel_near(*pixels, 46, 46, {0, 0, 0, 255})
               ? 0
               : 1;
  }
}
