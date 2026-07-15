#include "metal_pixel_test_support.hpp"

#include <span>

int main() {
  @autoreleasepool {
    cgpui_test::MetalPixelTestSurface surface;
    cgpui::RendererFrameDiagnosticSnapshot snapshot;
    auto pixels = cgpui_test::capture_metal_pixels(
        surface,
        cgpui::Color{.a = 1.0F},
        [](cgpui::RenderFrame& frame) {
          const cgpui::PaintMetadata translated{
              .opacity = 0.5F,
              .transform = cgpui::AffineTransform::translation(24.0F, 8.0F)};
          frame.draw_rect(cgpui::SolidRect{
              .rect = {.origin = {4.0F, 4.0F}, .size = {24.0F, 24.0F}},
              .color = {.r = 1.0F, .a = 1.0F},
              .clip_rect = cgpui::Rect{
                  .origin = {28.0F, 12.0F}, .size = {12.0F, 12.0F}},
              .composition_stack = cgpui::renderer_composition_stack_record(
                  std::span<const cgpui::PaintMetadata>(&translated, 1)),
              .metadata = translated});
          frame.draw_rounded_rect(cgpui::RoundedRectDraw{
              .rect = {.origin = {32.0F, 16.0F}, .size = {8.0F, 8.0F}},
              .color = {.b = 1.0F, .a = 1.0F}});
          frame.draw_rounded_rect(cgpui::RoundedRectDraw{
              .rect = {.origin = {32.0F, 16.0F}, .size = {8.0F, 8.0F}},
              .color = {.r = 1.0F, .g = 1.0F, .a = 1.0F}});
        },
        [&](cgpui::Renderer& renderer) {
          if (const auto* diagnostics = renderer.last_frame_diagnostic_snapshot()) {
            snapshot = *diagnostics;
          }
        });
    return pixels &&
                   cgpui_test::metal_pixel_near(*pixels, 20, 16, {0, 0, 0, 255}) &&
                   cgpui_test::metal_pixel_near(*pixels, 30, 14, {128, 0, 0, 255}, 5) &&
                   cgpui_test::metal_pixel_near(*pixels, 36, 20, {255, 255, 0, 255}) &&
                   cgpui_test::metal_pixel_near(*pixels, 44, 20, {0, 0, 0, 255}) &&
                   snapshot.work.exact_match() &&
                   snapshot.work.submitted_work.draw_count == 3 &&
                   snapshot.work.submitted_work.batch_count == 2
               ? 0
               : 1;
  }
}
