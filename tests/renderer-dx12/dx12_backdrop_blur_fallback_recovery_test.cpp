#include "dx12_backdrop_fallback_test_support.hpp"
#include "dx12_diagnostics_support.hpp"
#include "dx12_recovery_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool fallback_frame(Dx12BackdropFallbackFixture& fixture) {
  fixture.force_unsupported();
  auto frame = fixture.begin();
  if (!frame) return false;
  fallback_split_scene(*frame);
  frame->draw_backdrop_blur(fallback_panel());
  return fixture.capture(*frame) && fixture.pixel(24, 24, {128, 0, 0, 255}) &&
      fallback_diagnostics(fixture, 2, 2, 1, 1);
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12BackdropFallbackFixture fixture;
  if (!deadline.done || !fixture.initialize() || !fallback_frame(fixture))
    return dx12_fail(1, "fallback recovery real warmup failed");
  const auto before = fixture.state->info;
  const auto saved = *fixture.state->diagnostics;
  auto frame = fixture.begin();
  if (!frame) return dx12_fail(2, "fallback interrupted begin failed");
  fallback_split_scene(*frame);
  frame->draw_backdrop_blur(fallback_panel());
  if (!frame->request_pixel_capture() || !dx12_remove_native_device(before))
    return dx12_fail(3, "fallback active-frame native RemoveDevice failed");
  const auto result = frame->present();
  if (result || result.error().code != ErrorCode::frame_retry_required || fixture.state->pixels ||
      fixture.state->info.adapter_luid != before.adapter_luid ||
      fixture.state->info.device_generation != before.device_generation + 1 ||
      !diagnostic_same(fixture.state->diagnostics ? &*fixture.state->diagnostics : nullptr, saved))
    return dx12_fail(4, "fallback swallowed device loss or published an unsubmitted frame");
  const auto repeated = frame->present();
  if (repeated || repeated.error().code != ErrorCode::invalid_argument)
    return dx12_fail(5, "interrupted fallback frame was accepted twice");
  if (!fallback_frame(fixture) || !dx12_recovered_native_device_clean(fixture.state->info))
    return dx12_fail(6, "real recovered fallback pixels/debug state failed");
  return 0;
}
