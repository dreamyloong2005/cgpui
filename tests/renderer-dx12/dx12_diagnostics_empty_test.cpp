#include "../support/frame_wait_test_support.hpp"
#include "dx12_diagnostics_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(1, "empty diagnostics renderer unavailable");
  auto frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!frame) return dx12_fail(2, "empty diagnostics frame unavailable");
  const auto image = diagnostic_image();
  (*frame)->upload_image(image);
  (*frame)->clear({0, 0, 0, 1});
  for (const bool clipped : {false, true}) {
    const Rect bounds = clipped ? Rect{{4, 4}, {12, 12}} : Rect{};
    const std::optional<Rect> clip = clipped ? std::optional<Rect>{Rect{{80, 80}, {8, 8}}} : std::nullopt;
    (*frame)->draw_rect({.rect = bounds, .color = {1, 0, 0, 1}, .clip_rect = clip});
    (*frame)->draw_rounded_rect({.rect = bounds, .color = {0, 1, 0, 1}, .radius = BorderRadii::all(2), .clip_rect = clip});
    (*frame)->draw_text_selection({.rect = bounds, .color = {0, 0, 1, 1}, .clip_rect = clip});
    (*frame)->draw_text_caret({.rect = bounds, .color = {1, 1, 0, 1}, .clip_rect = clip});
    // The shared image builder emits even a zero-area quad. An empty scissor,
    // unlike zero bounds alone, prevents its native draw and lazy upload.
    (*frame)->draw_image({.bounds = bounds, .asset = describe_image_asset(image),
        .clip_rect = clipped ? clip : std::optional<Rect>{Rect{}}});
    auto text = diagnostic_text(clipped ? "M" : "", 16, {4, 4});
    text.clip_rect = clip;
    (*frame)->draw_text(text);
    (*frame)->draw_box_shadow({.rect = bounds, .shadow = {.color = {0, 1, 1, 1}, .offset = {16, 0}}, .clip_rect = clip});
    (*frame)->draw_backdrop_blur({.rect = bounds, .blur_radius = 0, .tint = Color{1, 0, 0, 1}, .clip_rect = clip});
  }
  if (!(*frame)->request_pixel_capture() || !(*frame)->present()) return dx12_fail(3, "empty diagnostic commands were rejected");
  const auto* pixels = (*renderer)->last_frame_pixels();
  if (!pixels || !pixels->valid()) return dx12_fail(4, "empty diagnostic capture absent");
  for (unsigned y = 0; y != pixels->height; ++y) for (unsigned x = 0; x != pixels->width; ++x)
    if (!dx12_pixel(pixels, x, y, {0, 0, 0, 255})) return dx12_fail(5, "empty/clipped command changed pixels");
  const auto* snapshot = (*renderer)->last_frame_diagnostic_snapshot();
  if (!diagnostic_work(snapshot, 16, 0, 0) || snapshot->upload_bytes.total_byte_count != 0 ||
      snapshot->backdrop_blur.blurred_backdrop_count || snapshot->backdrop_blur.offscreen_pass_count ||
      snapshot->backdrop_blur.offscreen_targets_allocated)
    return dx12_fail(6, "empty/clipped commands must not become submitted resources, uploads or dropped errors");
  auto degenerate = cgpui_test::wait_for_test_frame(**renderer);
  if (!degenerate) return dx12_fail(7, "degenerate image frame unavailable");
  (*degenerate)->clear({0, 0, 0, 1});
  (*degenerate)->draw_image({.bounds = {}, .asset = describe_image_asset(image)});
  if (!(*degenerate)->request_pixel_capture() || !(*degenerate)->present())
    return dx12_fail(8, "degenerate image present failed");
  snapshot = (*renderer)->last_frame_diagnostic_snapshot();
  if (!diagnostic_work(snapshot, 1, 1, 1) || snapshot->submitted_draws.image_count != 1 ||
      snapshot->upload_bytes.image_byte_count != 16)
    return dx12_fail(9, "diagnostics must count an emitted degenerate quad, not infer pixel coverage");
  pixels = (*renderer)->last_frame_pixels();
  if (!pixels || !pixels->valid()) return dx12_fail(10, "degenerate image capture absent");
  for (unsigned y = 0; y != pixels->height; ++y) for (unsigned x = 0; x != pixels->width; ++x)
    if (!dx12_pixel(pixels, x, y, {0, 0, 0, 255})) return dx12_fail(11, "degenerate quad changed pixels");
  return 0;
}
