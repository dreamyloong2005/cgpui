#include "renderer_backdrop_pass_test_support.hpp"
#include "renderer_pixel_test_support.hpp"

#include <iostream>

namespace cgpui_test {
namespace {
void stripes(cgpui::RenderFrame& frame) {
  for (int x = 0; x < 128; x += 4)
    frame.draw_rect({.rect = {{static_cast<float>(x), 0}, {2, 128}}, .color = {0, 0, 0, 1}});
}
cgpui::BackdropBlurDraw backdrop(cgpui::Rect rect) {
  return {.rect = rect, .radius = cgpui::BorderRadii::all(12.0F), .blur_radius = 4.0F};
}
} // namespace

int check_backdrop_pass_counts(cgpui::Renderer& renderer,
    unsigned effects, unsigned expected_passes, unsigned frames) {
  if (!renderer.backdrop_blur_support().supported) {
    report_pixel_test_error("diagnostic backdrop capability",
        {cgpui::ErrorCode::unsupported_platform, renderer.backdrop_blur_support().reason});
    return 1;
  }
  for (unsigned index = 0; index < frames; ++index) {
    auto frame = begin_pixel_capture(renderer);
    if (!frame) return 2;
    (*frame)->clear({1, 1, 1, 1});
    stripes(**frame);
    if (effects == 1) (*frame)->draw_backdrop_blur(backdrop({{32, 32}, {64, 64}}));
    if (effects == 2) {
      (*frame)->draw_backdrop_blur(backdrop({{8, 8}, {48, 48}}));
      (*frame)->draw_rect({.rect = {{88, 88}, {16, 16}}, .color = {0, 0, 0, 1}});
      (*frame)->draw_backdrop_blur(backdrop({{72, 72}, {48, 48}}));
    }
    if (!pixel_test_ok((*frame)->present(), "diagnostic present")) return 3;
  }
  if (!completed_pixel_capture(renderer)) return 4;
  const auto* snapshot = renderer.last_frame_diagnostic_snapshot();
  if (!snapshot) return 5;
  const auto& blur = snapshot->backdrop_blur;
  std::cout << "backdrop topology: backend=" << pixel_backend_name(renderer.backend_info().backend)
            << " effects=" << effects << " passes=" << blur.offscreen_pass_count
            << " expected=" << expected_passes << '\n';
  return blur.blurred_backdrop_count == effects && blur.offscreen_pass_count == expected_passes &&
         blur.offscreen_targets_allocated == (effects != 0) &&
         snapshot->submitted_draws.backdrop_blur_count == effects ? 0 : 6;
}
} // namespace cgpui_test
