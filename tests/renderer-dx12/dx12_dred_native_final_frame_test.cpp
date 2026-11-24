#include "dx12_dred_native_test_support.hpp"
#include "../support/frame_wait_test_support.hpp"

int main() {
  using namespace cgpui; using namespace cgpui_test;
  Dx12RecoveryDeadline deadline;
  Dx12Window window;
  DredNativeReadout readout; // Must outlive facade, sole frame and state destructor.
  if (!deadline.done || !window.hwnd || !dred_select_debug(true)) return dx12_fail(1, "final-frame setup");
  auto made = create_dx12_renderer(window.descriptor());
  if (!made) return dx12_fail(2, "final-frame actual public factory");
  auto renderer = std::move(*made);
  const auto info = renderer->backend_info();
  if (!dred_record_identity(info, "final-frame-initial")) return dx12_fail(10, "final-frame native identity/output failed");
  const auto weak = dred_attach_public_readout(info, window.hwnd, readout);
  auto frame = wait_for_test_frame(*renderer);
  if (weak.expired() || !frame || readout.calls) return dx12_fail(3, "final-frame acquired owner/readout");
  const auto loss = dred_remove_actual(info, true);
  if (!loss.valid) return dx12_fail(4, "final-frame actual idle removal failed");
  renderer.reset(); // Actual facade destructor, no synthetic owner_alive flag.
  if (weak.use_count() != 1 || readout.calls || !IsWindow(window.hwnd))
    return dx12_fail(5, "facade removal retained extra state owner or prematurely retired frame");
  frame->reset(); // No Present/recovery: this sole frame triggers final state native cleanup.
  if (!weak.expired() || readout.calls != 1 || !dred_current_readout(readout, loss, info.device_generation))
    return dx12_fail(6, "final frame destructor missed actual current scalar before native release");
  // Only readout scalars survive; do not dereference the expired state or hold COM.
  auto fresh = create_dx12_renderer(window.descriptor());
  if (!fresh) return dx12_fail(7, "final-frame diagnostic reference prevented singleton replacement");
  if (!dred_record_identity((*fresh)->backend_info(), "final-frame-fresh"))
    return dx12_fail(10, "fresh native identity/output failed");
  auto next = wait_for_test_frame(**fresh);
  if (!next) return dx12_fail(8, "fresh renderer after final-frame teardown unavailable");
  (*next)->clear({0,1,0,1});
  if (!(*next)->request_pixel_capture() || !(*next)->present() ||
      !dx12_pixel((*fresh)->last_frame_pixels(), 32, 32, {0,255,0,255}))
    return dx12_fail(9, "fresh actual pixels after final-frame native cleanup failed");
  return dred_flush_output() ? 0 : dx12_fail(11, "final-frame evidence output flush failed");
}
