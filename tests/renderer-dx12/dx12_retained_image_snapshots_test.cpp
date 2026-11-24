#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"
#include "dx12_image_retention_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(1, "retained immutable snapshots renderer unavailable");
  if ((*renderer)->last_frame_diagnostic_snapshot()) return dx12_fail(2, "new renderer published history");
  const auto red = retention_image(9820, {255, 0, 0, 255});
  const auto green = retention_image(9820, {0, 255, 0, 255});
  const auto asset = describe_image_asset(red);
  auto finished = wait_for_test_frame(**renderer);
  if (!finished) return dx12_fail(3, "immutable snapshots frame unavailable");
  (*finished)->clear({0, 0, 1, 1});
  (*finished)->upload_image(red);
  (*finished)->draw_image({.bounds = {{0, 0}, {24, 64}}, .asset = asset});
  (*finished)->upload_image(green);
  (*finished)->draw_image({.bounds = {{32, 0}, {24, 64}}, .asset = asset});
  (*finished)->invalidate_image(asset.id);
  if (!(*finished)->request_pixel_capture() || !(*finished)->present() ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 8, 8, {255, 0, 0, 255}) ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 40, 8, {0, 255, 0, 255}) ||
      !retention_sample(**renderer, 8, 8))
    return dx12_fail(4, "same-id immutable pixels or unique retained payload were lost after invalidation");
  // Cache is empty. The finished frame, not the submission vector after the next
  // frame starts, must keep both immutable CPU snapshots observable.
  if (!retention_empty_frame(**renderer, 8))
    return dx12_fail(5, "finished frame's payload was confused with current cache residency");
  finished->reset();
  if (!retention_empty_frame(**renderer, 0))
    return dx12_fail(6, "final immutable owner release did not appear in the next successful sample");
  return 0;
}
