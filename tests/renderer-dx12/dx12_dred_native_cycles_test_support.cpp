#include "dx12_dred_native_test_support.hpp"
#include "dx12_diagnostics_support.hpp"
#include "../support/frame_wait_test_support.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"

namespace cgpui_test {
namespace {
bool paint(cgpui::Renderer& renderer, cgpui::Color color,
           std::array<unsigned char, 4> expected) {
  auto frame = wait_for_test_frame(renderer);
  if (!frame) return false;
  (*frame)->clear(color);
  (*frame)->draw_rect({.rect = {{4,4},{16,16}}, .color = {0,1,0,1}});
  return (*frame)->request_pixel_capture() && (*frame)->present() &&
      dx12_pixel(renderer.last_frame_pixels(), 8, 8, {0,255,0,255}) &&
      dx12_pixel(renderer.last_frame_pixels(), 40, 40, expected);
}
}
int dred_native_cycles(bool debug) {
  using namespace cgpui;
  Dx12RecoveryDeadline deadline;
  Dx12Window window;
  DredNativeReadout readout; // Outlives the public facade and all native teardown.
  if (!deadline.done || !window.hwnd || !dred_select_debug(debug)) return dx12_fail(1, "DRED cycles setup");
  auto made = create_dx12_renderer(window.descriptor());
  if (!made) return dx12_fail(2, "DRED cycles actual public factory");
  auto renderer = std::move(*made);
  const auto initial = renderer->backend_info();
  if (!dred_record_identity(initial, "cycles-initial")) return dx12_fail(13, "actual native identity/output failed");
  const auto weak = dred_attach_public_readout(initial, window.hwnd, readout);
  if (weak.expired() || !paint(*renderer, {1,0,0,1}, {255,0,0,255}))
    return dx12_fail(3, "DRED cycles initial actual pixels/weak state");
  for (unsigned cycle = 0; cycle != 2; ++cycle) {
    const auto before = renderer->backend_info();
    const auto old_diagnostics = renderer->last_frame_diagnostic_snapshot();
    if (!old_diagnostics) return dx12_fail(4, "DRED cycles successful frame history absent");
    const auto saved_diagnostics = *old_diagnostics;
    const auto loss = dred_remove_actual(before, debug);
    if (!loss.valid) return dx12_fail(5, "DRED cycles actual idle native removal not established");
    const auto retry = renderer->begin_frame(); // Raw public call: do not hide the first retry.
    if (retry || retry.error().code != ErrorCode::frame_retry_required)
      return dx12_fail(6, "DRED cycles changed original public retry contract");
    const auto after = renderer->backend_info();
    if (!dred_record_identity(after, "cycles-recovered")) return dx12_fail(13, "recovered native identity/output failed");
    if (after.device_generation != initial.device_generation + cycle + 1 ||
        after.adapter_luid != initial.adapter_luid || renderer->last_frame_pixels())
      return dx12_fail(7, "DRED cycles changed generation/adapter/capture invalidation");
    if (readout.calls != cycle + 1 || !dred_current_readout(readout, loss, before.device_generation))
      return dx12_fail(8, "DRED common hook failed current scalar publication after actual loss");
    { const auto state = weak.lock();
      if (!state) return dx12_fail(9, "DRED cycles state unexpectedly expired");
      Dx12OperationGuard gate(*state);
      const auto view = state->dred_status.snapshot_locked();
      if (view.phase != Dx12DredSitePhase::open || view.last_is_current || !view.last_loss ||
          !dred_status_matches(*view.last_loss, loss) || !state->device ||
          FAILED(state->device->GetDeviceRemovedReason()))
        return dx12_fail(10, "replacement did not open local opportunity/preserve historical scalar");
    }
    if (!diagnostic_same(renderer->last_frame_diagnostic_snapshot(), saved_diagnostics))
      return dx12_fail(11, "scalar loss overwrote last-successful frame diagnostics");
    if (!paint(*renderer, cycle ? Color{0,1,0,1} : Color{0,0,1,1},
               cycle ? std::array<unsigned char,4>{0,255,0,255} : std::array<unsigned char,4>{0,0,255,255}) ||
        !dx12_recovered_native_device_clean(after))
      return dx12_fail(12, "DRED cycles post-recovery actual pixels/native debug");
  }
  return dred_flush_output() ? 0 : dx12_fail(14, "native evidence output flush failed");
}
} // namespace cgpui_test
