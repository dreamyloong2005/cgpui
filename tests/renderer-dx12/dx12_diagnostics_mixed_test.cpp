#include "../support/frame_wait_test_support.hpp"
#include "dx12_diagnostics_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(1, "mixed diagnostics renderer unavailable");
  auto frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!frame) return dx12_fail(2, "mixed diagnostics frame unavailable");
  const auto image = diagnostic_image();
  (*frame)->clear({0, 0, 0, 1});
  (*frame)->upload_image(image);
  (*frame)->draw_rect({.rect = {{0, 0}, {12, 12}}, .color = {1, 0, 0, 1}});
  (*frame)->draw_rounded_rect({.rect = {{16, 0}, {12, 12}}, .color = {0, 1, 0, 1}, .radius = BorderRadii::all(2)});
  (*frame)->draw_text_selection({.rect = {{32, 0}, {12, 12}}, .color = {0, 0, 1, 1}});
  (*frame)->draw_text_caret({.rect = {{48, 0}, {4, 12}}, .color = {1, 1, 0, 1}});
  (*frame)->draw_image({.bounds = {{0, 16}, {12, 12}}, .asset = describe_image_asset(image),
      .sampling = ImageSamplingMode::nearest});
  (*frame)->draw_text(diagnostic_text("M", 16, {16, 16}));
  (*frame)->draw_box_shadow({.rect = {{80, 80}, {12, 12}},
      .shadow = {.color = {0, 1, 1, 1}, .offset = {-44, -64}}});
  (*frame)->draw_backdrop_blur({.rect = {{48, 40}, {12, 12}}, .blur_radius = 0,
      .tint = Color{1, 0, 0, 0.5F}});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present()) return dx12_fail(3, "mixed diagnostics present failed");
  const auto* pixels = (*renderer)->last_frame_pixels();
  if (!dx12_pixel(pixels, 4, 4, {255, 0, 0, 255}) || !dx12_pixel(pixels, 22, 6, {0, 255, 0, 255}) ||
      !dx12_pixel(pixels, 36, 4, {0, 0, 255, 255}) || !dx12_pixel(pixels, 49, 4, {255, 255, 0, 255}) ||
      !dx12_pixel(pixels, 2, 18, {255, 0, 0, 255}) || !dx12_pixel(pixels, 9, 25, {255, 255, 255, 255}) ||
      !dx12_pixel(pixels, 40, 20, {0, 255, 255, 255}) || !dx12_pixel(pixels, 54, 46, {128, 0, 0, 255}) ||
      !diagnostic_ink(pixels, 16, 16, 34, 40)) return dx12_fail(4, "mixed diagnostic primitives have missing pixels");
  const auto* snapshot = (*renderer)->last_frame_diagnostic_snapshot();
  const RendererDrawCounts expected{.solid_rect_count = 1, .rounded_rect_count = 1, .text_count = 1,
      .text_selection_count = 1, .text_caret_count = 1, .image_count = 1,
      .box_shadow_count = 1, .backdrop_blur_count = 1, .total_count = 8};
  if (!diagnostic_work(snapshot, 8, 8, 8) || diagnostic_counts(snapshot->submitted_draws) != diagnostic_counts(expected) ||
      snapshot->backdrop_blur.blurred_backdrop_count != 1 || snapshot->backdrop_blur.offscreen_pass_count != 3 ||
      !snapshot->backdrop_blur.offscreen_targets_allocated || snapshot->upload_bytes.image_byte_count != 16 ||
      snapshot->upload_bytes.glyph_atlas_byte_count == 0)
    return dx12_fail(5, "mixed diagnostic categories, payload bytes, timings or blur passes differ");
  return 0;
}
