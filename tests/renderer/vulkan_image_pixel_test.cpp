#include "vulkan_pixel_test_support.hpp"

#include <cstdio>

int main() {
  cgpui_test::VulkanPixelTestWindow window(
      L"CGPUIVulkanImagePixelTestWindow", L"CGPUI Vulkan Image Pixel Test");
  cgpui::ImageAsset image{
      .id = {524},
      .logical_size = {2.0F, 2.0F},
      .bitmap = {
          .width = 2,
          .height = 2,
          .stride = 8,
          .format = cgpui::ImageFormat::rgba8_unorm,
          .pixels = {
              255, 0, 0, 255, 0, 255, 0, 255,
              0, 0, 255, 255, 255, 255, 255, 255,
          },
      },
  };
  const cgpui::ImageAssetDescriptor descriptor =
      cgpui::describe_image_asset(image);
  auto pixels = cgpui_test::capture_pixels(
      window,
      {.a = 1.0F},
      [&](cgpui::RenderFrame& frame) {
        frame.upload_image(image);
        frame.draw_image(cgpui::ImageDraw{
            .bounds = {.origin = {8.0F, 8.0F}, .size = {40.0F, 40.0F}},
            .asset = descriptor,
            .sampling = cgpui::ImageSamplingMode::nearest,
        });
      });
  if (!pixels) {
    return 1;
  }
  const auto top_left = pixels->pixel_rgba8(18, 18);
  const auto top_right = pixels->pixel_rgba8(38, 18);
  const auto bottom_left = pixels->pixel_rgba8(18, 38);
  const auto bottom_right = pixels->pixel_rgba8(38, 38);
  const bool matches =
      cgpui_test::pixel_near(*pixels, 18, 18, {255, 0, 0, 255}) &&
      cgpui_test::pixel_near(*pixels, 38, 18, {0, 255, 0, 255}) &&
      cgpui_test::pixel_near(*pixels, 18, 38, {0, 0, 255, 255}) &&
      cgpui_test::pixel_near(*pixels, 38, 38, {255, 255, 255, 255});
  if (!matches && top_left && top_right && bottom_left && bottom_right) {
    std::fprintf(
        stderr,
        "image samples tl=%u,%u,%u,%u tr=%u,%u,%u,%u "
        "bl=%u,%u,%u,%u br=%u,%u,%u,%u\n",
        (*top_left)[0], (*top_left)[1], (*top_left)[2], (*top_left)[3],
        (*top_right)[0], (*top_right)[1], (*top_right)[2], (*top_right)[3],
        (*bottom_left)[0], (*bottom_left)[1], (*bottom_left)[2],
        (*bottom_left)[3], (*bottom_right)[0], (*bottom_right)[1],
        (*bottom_right)[2], (*bottom_right)[3]);
  }
  return matches ? 0 : 2;
}
