#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"
#include "dx12_image_retention_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(1, "image batch validation renderer unavailable");
  const auto red = retention_image(9882, {255, 0, 0, 255});
  const auto green = retention_image(9882, {0, 255, 0, 255});
  const auto asset = describe_image_asset(red);
  const auto generation = (*renderer)->backend_info().device_generation;
  {
    auto frame = wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(2, "final-upload validation frame unavailable");
    auto invalid = red; invalid.bitmap.stride = 0;
    (*frame)->clear({0, 0, 0, 1});
    (*frame)->draw_image({.bounds = {{0, 0}, {64, 64}}, .asset = asset}); // Valid upload follows the draw.
    (*frame)->upload_image(invalid);
    (*frame)->upload_image(green); // Only this valid final same-ID upload is prepared.
    if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
        !dx12_pixel((*renderer)->last_frame_pixels(), 32, 32, {0, 255, 0, 255}) ||
        !retention_sample(**renderer, 4, 4))
      return dx12_fail(3, "deferred valid final upload/caller draw did not resolve once");
  }
  {
    auto frame = wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(4, "invalid-final frame unavailable");
    auto invalid = red; invalid.bitmap.stride = 0;
    (*frame)->upload_image(red); (*frame)->upload_image(invalid);
    const auto failed = (*frame)->present(); // Raw input failure; never retry this frame.
    if (failed || failed.error().code != ErrorCode::invalid_argument)
      return dx12_fail(5, "malformed final upload fell back to an earlier valid value");
  }
  auto healthy = wait_for_test_frame(**renderer);
  if (!healthy || (*renderer)->backend_info().device_generation != generation)
    return dx12_fail(6, "CPU image preparation rejection poisoned or rebuilt the live renderer");
  (*healthy)->upload_image(red); // Resupply explicitly; no arbitrary failed-Present rollback promise.
  if (!retention_present_image(**renderer, **healthy, asset, {{255, 0, 0, 255}, 4, 4}))
    return dx12_fail(7, "healthy replacement after input failure did not draw/copy normally");
  return 0;
}
