#include "metal_pixel_test_support.hpp"

int main() {
  @autoreleasepool {
    const cgpui::FontDescriptor font{.family = "Inter"};
    const std::string content = "A";
    const cgpui::TextShapeRun shaped = cgpui::shape_text(content, font, 20.0F);
    cgpui::ImageAsset image{
        .id = {7002},
        .logical_size = {2.0F, 2.0F},
        .bitmap = {
            .width = 2,
            .height = 2,
            .stride = 8,
            .pixels = {255, 0, 0, 255, 0, 255, 0, 255,
                       0, 0, 255, 255, 255, 255, 255, 255}}};
    const cgpui::SvgRasterizationResult svg = cgpui::rasterize_svg(
        cgpui::SvgRasterizationRequest{
            .asset_id = {7003},
            .logical_size = {3.0F, 2.0F},
            .svg_source =
                "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"3\" "
                "height=\"2\"><rect width=\"3\" height=\"2\" "
                "fill=\"#ff00ff\"/></svg>",
            .scale = {1.0F}});
    if (!svg.ready()) return 1;
    const cgpui::TextDraw text{
        .bounds = {.origin = {4.0F, 4.0F}, .size = {24.0F, 24.0F}},
        .color = {.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
        .font = font,
        .content = content,
        .byte_length = content.size(),
        .font_size = 20.0F,
        .scale = {1.0F},
        .device_font_size = 20.0F,
        .glyphs = cgpui::text_glyph_paint_metadata(shaped, {4.0F, 4.0F})};
    const cgpui::TextShapeRun scaled_shape =
        cgpui::shape_text(content, font, 20.0F, cgpui::DpiScale{2.0F});
    const cgpui::TextDraw scaled_text{
        .bounds = {.origin = {2.0F, 2.0F}, .size = {24.0F, 24.0F}},
        .color = {.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
        .font = font,
        .content = content,
        .byte_length = content.size(),
        .font_size = 20.0F,
        .scale = {2.0F},
        .device_font_size = 40.0F,
        .glyphs = cgpui::text_glyph_paint_metadata(scaled_shape, {2.0F, 2.0F})};
    cgpui_test::MetalPixelTestSurface surface;
    auto pixels = cgpui_test::capture_metal_pixels(
        surface,
        cgpui::Color{.a = 1.0F},
        [&](cgpui::RenderFrame& frame) {
          frame.draw_text(text);
          frame.upload_image(image);
          frame.draw_image(cgpui::ImageDraw{
              .bounds = {.origin = {32.0F, 8.0F}, .size = {24.0F, 24.0F}},
              .asset = cgpui::describe_image_asset(image),
              .sampling = cgpui::ImageSamplingMode::nearest});
          frame.upload_image(svg.image);
          frame.draw_image(cgpui::ImageDraw{
              .bounds = {.origin = {32.0F, 40.0F}, .size = {24.0F, 16.0F}},
              .asset = cgpui::describe_image_asset(svg.image),
              .sampling = cgpui::ImageSamplingMode::nearest});
        });
    if (!pixels || !pixels->valid()) return 1;
    std::size_t glyph_pixels = 0;
    for (std::uint32_t y = 4; y < 28; ++y) {
      for (std::uint32_t x = 4; x < 28; ++x) {
        const auto pixel = pixels->pixel_rgba8(x, y);
        if (pixel && (*pixel)[0] > 200 && (*pixel)[1] > 200 &&
            (*pixel)[2] > 200) ++glyph_pixels;
      }
    }
    if (glyph_pixels < 20 ||
        !cgpui_test::metal_pixel_near(*pixels, 38, 14, {255, 0, 0, 255}) ||
        !cgpui_test::metal_pixel_near(*pixels, 50, 14, {0, 255, 0, 255}) ||
        !cgpui_test::metal_pixel_near(*pixels, 38, 26, {0, 0, 255, 255}) ||
        !cgpui_test::metal_pixel_near(*pixels, 50, 26, {255, 255, 255, 255}) ||
        !cgpui_test::metal_pixel_near(*pixels, 44, 48, {255, 0, 255, 255})) {
      return 2;
    }

    auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
        .native_surface = surface.surface(),
        .framebuffer_size = {64.0F, 64.0F},
        .scale = {1.0F}});
    if (!renderer) return 3;
    auto first = (*renderer)->begin_frame();
    if (!first) return 4;
    (*first)->draw_text(text);
    (*first)->upload_image(image);
    if (!(*first)->present()) return 5;
    const auto* first_snapshot = (*renderer)->last_frame_diagnostic_snapshot();
    if (first_snapshot == nullptr ||
        first_snapshot->upload_bytes.glyph_atlas_byte_count == 0 ||
        first_snapshot->upload_bytes.image_byte_count != image.bitmap.pixels.size()) {
      return 6;
    }
    (*first).reset();

    auto second = (*renderer)->begin_frame();
    if (!second) return 7;
    (*second)->draw_text(text);
    if (!(*second)->present()) return 8;
    const auto* second_snapshot = (*renderer)->last_frame_diagnostic_snapshot();
    if (second_snapshot == nullptr ||
        second_snapshot->upload_bytes.glyph_atlas_byte_count != 0) return 9;
    (*second).reset();

    auto scaled = (*renderer)->begin_frame();
    if (!scaled || !(*scaled)->request_pixel_capture()) return 10;
    (*scaled)->clear(cgpui::Color{.a = 1.0F});
    (*scaled)->draw_text(scaled_text);
    if (!(*scaled)->present()) return 11;
    const auto* scaled_pixels = (*renderer)->last_frame_pixels();
    if (scaled_pixels == nullptr ||
        !cgpui_test::metal_pixel_near(
            *scaled_pixels, 16, 24, {255, 255, 255, 255})) return 12;
    (*scaled).reset();

    auto invalidated = (*renderer)->begin_frame();
    if (!invalidated) return 13;
    (*invalidated)->invalidate_image(image.id);
    (*invalidated)->draw_image(cgpui::ImageDraw{
        .bounds = {.origin = {4.0F, 4.0F}, .size = {16.0F, 16.0F}},
        .asset = cgpui::describe_image_asset(image)});
    return (*invalidated)->present() ? 14 : 0;
  }
}
