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
  auto old_red = wait_for_test_frame(**renderer);
  if (!old_red) return dx12_fail(3, "old immutable red frame unavailable");
  (*old_red)->upload_image(red);
  if (!retention_present_image(**renderer, **old_red, asset, {{255, 0, 0, 255}, 4, 4}))
    return dx12_fail(4, "old immutable red pixels or payload absent");
  auto old_green = wait_for_test_frame(**renderer);
  if (!old_green) return dx12_fail(5, "replacement immutable green frame unavailable");
  (*old_green)->upload_image(green);
  if (!retention_present_image(**renderer, **old_green, asset, {{0, 255, 0, 255}, 8, 4}))
    return dx12_fail(6, "same-ID cross-frame replacement lost the still-owned red snapshot");
  {
    auto invalidation = wait_for_test_frame(**renderer);
    if (!invalidation) return dx12_fail(7, "invalidation-only retained frame unavailable");
    (*invalidation)->invalidate_image(asset.id);
    (*invalidation)->clear({0, 0, 1, 1});
    if (!(*invalidation)->request_pixel_capture() || !(*invalidation)->present() ||
        !dx12_pixel((*renderer)->last_frame_pixels(), 32, 32, {0, 0, 255, 255}) ||
        !retention_sample(**renderer, 8, 0))
      return dx12_fail(8, "cache invalidation subtracted payload still owned by finished frames");
  }
  // Subsequent frame-start cleanup drops submission references; the two finished
  // frames, not current cache residency, must still retain both immutable images.
  if (!retention_empty_frame(**renderer, 8)) return dx12_fail(9, "old frame payload omitted after submission cleanup");
  old_red->reset();
  if (!retention_empty_frame(**renderer, 4)) return dx12_fail(10, "old red final-owner release was not sampled");
  old_green->reset();
  if (!retention_empty_frame(**renderer, 0)) return dx12_fail(11, "old green final-owner release was not sampled");
  return 0;
}
