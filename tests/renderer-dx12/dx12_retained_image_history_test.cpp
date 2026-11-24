#include "../support/frame_wait_test_support.hpp"
#include "dx12_diagnostics_support.hpp"
#include "dx12_image_retention_test_support.hpp"
#include <limits>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(1, "retained history renderer unavailable");
  const auto red = retention_image(9830, {255, 0, 0, 255});
  const auto green = retention_image(9831, {0, 255, 0, 255});
  const auto blue = retention_image(9832, {0, 0, 255, 255});
  {
    auto abandoned = wait_for_test_frame(**renderer);
    if (!abandoned) return dx12_fail(2, "initial abandoned frame unavailable");
    (*abandoned)->upload_image(red);
  }
  if ((*renderer)->last_frame_diagnostic_snapshot() || !retention_empty_frame(**renderer, 0))
    return dx12_fail(3, "abandonment published or applied its pending image bytes");
  {
    auto seed = wait_for_test_frame(**renderer);
    if (!seed) return dx12_fail(9, "successful retained-history seed unavailable");
    (*seed)->upload_image(red);
    if (!retention_present_image(**renderer, **seed, describe_image_asset(red), {{255, 0, 0, 255}, 4, 4}))
      return dx12_fail(10, "successful retained-history seed pixels/payload absent");
  }
  const auto* borrowed = (*renderer)->last_frame_diagnostic_snapshot();
  const auto saved = *borrowed;
  const auto unchanged = [&] {
    return (*renderer)->last_frame_diagnostic_snapshot() == borrowed && diagnostic_same(borrowed, saved);
  };
  auto invalid = wait_for_test_frame(**renderer);
  if (!invalid) return dx12_fail(4, "failed retained-image frame unavailable");
  (*invalid)->upload_image(green);
  (*invalid)->clear({std::numeric_limits<float>::quiet_NaN(), 0, 0, 1});
  const auto failed = (*invalid)->present(); // Known early rejection: the image prefix never runs.
  if (failed || failed.error().code != ErrorCode::invalid_argument || !unchanged())
    return dx12_fail(5, "failed frame rewrote the previous successful retained payload");
  invalid->reset();
  {
    auto abandoned = wait_for_test_frame(**renderer);
    if (!abandoned) return dx12_fail(6, "later abandoned retained-image frame unavailable");
    (*abandoned)->upload_image(blue);
  }
  if (!unchanged() || !(*renderer)->resize({96, 64}, {1}) || !unchanged() ||
      (*renderer)->resize({0, 64}, {1}) || !unchanged() ||
      !(*renderer)->resize({64, 64}, {1}) || !unchanged())
    return dx12_fail(7, "abandonment/resize changed historical retained count or saturation");
  if (!retention_empty_frame(**renderer, 4))
    return dx12_fail(8, "abandoned/early-rejected pending bytes changed the next successful census");
  return 0;
}
