#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"
#include "dx12_image_retention_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(1, renderer.error().message.c_str());
  const ImageAsset cold{.id = {70}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 0, 0, 255}}};
  auto warm = cold;
  warm.id = {71};
  {
    auto frame = cgpui_test::wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(2, "cache seed begin failed");
    (*frame)->upload_image(cold);
    (*frame)->upload_image(warm);
    (*frame)->draw_image({.bounds = {{0, 0}, {16, 16}}, .asset = describe_image_asset(cold)});
    (*frame)->draw_image({.bounds = {{16, 0}, {16, 16}}, .asset = describe_image_asset(warm)});
    if (auto result = (*frame)->present(); !result) return dx12_fail(3, result.error().message.c_str());
    if (!retention_sample(**renderer, 8)) return dx12_fail(16, "seed retention must include both CPU images");
  }
  for (unsigned idle = 0; idle < 120; ++idle) {
    auto frame = cgpui_test::wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(4, "idle cache frame failed");
    if (auto result = (*frame)->present(); !result) return dx12_fail(5, result.error().message.c_str());
    if (!retention_sample(**renderer, 8, 0)) return dx12_fail(17, "idle success evicted or counted upload too early");
  }
  {
    // Exactly 120 completed idle frames retain the upload. Touching warm now
    // refreshes it while cold exceeds the idle limit on this presentation.
    auto frame = cgpui_test::wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(6, "boundary frame failed");
    (*frame)->draw_image({.bounds = {{0, 0}, {16, 16}}, .asset = describe_image_asset(warm)});
    if (!(*frame)->request_pixel_capture()) return dx12_fail(7, "boundary capture failed");
    if (auto result = (*frame)->present(); !result) return dx12_fail(8, result.error().message.c_str());
    if (!dx12_pixel((*renderer)->last_frame_pixels(), 8, 8, {255, 0, 0, 255}))
      return dx12_fail(9, "cache evicted at or before 120 idle frames");
    if (!retention_sample(**renderer, 4, 0))
      return dx12_fail(18, "121st idle success published retention before this frame's cold eviction");
  }
  {
    auto frame = cgpui_test::wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(10, "evicted frame failed");
    (*frame)->draw_image({.bounds = {{0, 0}, {16, 16}}, .asset = describe_image_asset(cold)});
    (*frame)->clear({0, 1, 1, 1});
    if (!(*frame)->request_pixel_capture() || !(*frame)->present())
      return dx12_fail(11, "evicted image must skip without rejecting the frame");
    const auto* s = (*renderer)->last_frame_diagnostic_snapshot();
    if (!dx12_pixel((*renderer)->last_frame_pixels(), 8, 8, {0, 255, 255, 255}) || !s ||
        s->planned_draws.image_count != 1 || s->submitted_draws.image_count != 0 ||
        s->dropped_resources.missing_submission_resource_count != 1 || s->work.exact_match() ||
        !retention_sample(**renderer, 4, 0)) return dx12_fail(19, "eviction skip/copy/retention diagnostics differ");
  }
  auto frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!frame) return dx12_fail(12, "eviction error poisoned renderer");
  (*frame)->upload_image(cold);
  (*frame)->draw_image({.bounds = {{0, 0}, {16, 16}}, .asset = describe_image_asset(cold)});
  (*frame)->draw_image({.bounds = {{16, 0}, {16, 16}}, .asset = describe_image_asset(warm)});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(13, "reupload capture failed");
  if (auto result = (*frame)->present(); !result) return dx12_fail(14, result.error().message.c_str());
  return dx12_pixel((*renderer)->last_frame_pixels(), 8, 8, {255, 0, 0, 255}) &&
      dx12_pixel((*renderer)->last_frame_pixels(), 24, 8, {255, 0, 0, 255})
      && retention_sample(**renderer, 8, 4)
      ? 0 : dx12_fail(15, "reuploaded/touched cache pixels differ");
}
