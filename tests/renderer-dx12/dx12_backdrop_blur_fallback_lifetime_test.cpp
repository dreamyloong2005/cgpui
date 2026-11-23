#include "dx12_backdrop_fallback_test_support.hpp"
#include "dx12_diagnostics_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool supported_frame(Dx12BackdropFallbackFixture& fixture) {
  fixture.state->backdrop_blur->support = fixture.actual_support;
  auto frame = fixture.begin();
  if (!frame) return false;
  fallback_split_scene(*frame);
  auto panel = fallback_panel();
  panel.tint.reset();
  frame->draw_backdrop_blur(panel);
  if (!fixture.capture(*frame)) return false;
  const auto left = fixture.state->pixels->pixel_rgba8(28, 20);
  const auto right = fixture.state->pixels->pixel_rgba8(35, 20);
  if (!left || !right || (*left)[3] != 255 || (*right)[3] != 255) return false;
  // Independent Gaussian step bands; tint/no-op cannot satisfy these.
  for (unsigned channel = 0; channel != 3; ++channel) {
    if ((*left)[channel] < 24 || (*left)[channel] > 96 ||
        (*right)[channel] < 159 || (*right)[channel] > 231) return false;
  }
  const auto* report = fixture.state->diagnostics ? &*fixture.state->diagnostics : nullptr;
  return diagnostic_work(report, 2, 2, 2) && report->work.exact_match() &&
      report->submitted_draws.solid_rect_count == 1 &&
      report->submitted_draws.backdrop_blur_count == 1 &&
      report->submitted_draws.rounded_rect_count == 0 &&
      report->backdrop_blur.blurred_backdrop_count == 1 &&
      report->backdrop_blur.offscreen_pass_count == 3 &&
      report->backdrop_blur.offscreen_targets_allocated &&
      fixture.state->backdrop_blur->frame_targets.size() == 1;
}
} // namespace

int main() {
  Dx12BackdropFallbackFixture fixture;
  if (!fixture.initialize() || !fixture.actual_support.supported)
    return dx12_fail(1, "supported native blur control unavailable; this test does not skip");
  if (!supported_frame(fixture)) return dx12_fail(2, "initial supported blur pixel/control failed");
  for (const bool tint : {true, false}) {
    fixture.force_unsupported();
    auto frame = fixture.begin();
    if (!frame) return dx12_fail(3, "fallback lifetime begin failed");
    fallback_split_scene(*frame);
    auto panel = fallback_panel();
    if (!tint) panel.tint.reset();
    frame->draw_backdrop_blur(panel);
    if (!fixture.capture(*frame) ||
        !fixture.pixel(31, 24, tint ? std::array<unsigned char, 4>{128, 0, 0, 255}
                                 : std::array<unsigned char, 4>{0, 0, 0, 255}) ||
        !fallback_diagnostics(fixture, tint ? 2 : 1, tint ? 2 : 1, 1, tint ? 1 : 0))
      return dx12_fail(4, "fallback retained prior supported blur work or stale targets");
    if (!supported_frame(fixture)) return dx12_fail(5, "restored supported blur did not run its real plan");
  }
  return 0;
}
