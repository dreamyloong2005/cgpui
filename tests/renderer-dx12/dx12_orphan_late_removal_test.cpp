#include "../support/frame_wait_test_support.hpp"
#include "dx12_pacing_begin_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "dx12_orphan_native_removal_test_support.hpp"
#include "../../src/renderer/dx12/dx12_renderer_creation_internal.hpp"
#include <limits>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window window, peer_window;
  Dx12PacingForwardApi api;
  Dx12OrphanNativeRemoval removal; // Outlives both facade and frame/native-state cleanup.
  if (!deadline.done || !window.hwnd || !peer_window.hwnd || !pacing_begin_debug_enable())
    return dx12_fail(1, "late native orphan removal fixture unavailable");
  auto created = dx12_create_renderer(window.descriptor(), api, &removal);
  auto peer = create_dx12_renderer(peer_window.descriptor());
  if (!created || !peer) return dx12_fail(2, "late native removal real renderers unavailable");
  auto renderer = std::move(*created);
  const auto before = renderer->backend_info(), peer_before = (*peer)->backend_info();
  if (!before.adapter_luid || before.adapter_luid != peer_before.adapter_luid)
    return dx12_fail(3, "late removal must observe real same-adapter peers");
  auto orphan = pacing_begin_bounded(*renderer, api);
  if (!orphan || removal.signals || removal.removed_once) return dx12_fail(4, "late removal occurred before submission");
  const auto polls = api.polls;
  renderer.reset();
  if (!(*orphan)->request_pixel_capture()) return dx12_fail(5, "late removal orphan lost active capture permission");
  (*orphan)->clear({1, 0, 0, 1});
  const auto presented = (*orphan)->present(); // Healthy preflight, actual Present/Signal, then actual RemoveDevice.
  if (removal.signals != 1 || removal.removal_calls != 1 || FAILED(removal.signal_result) ||
      FAILED(removal.before_remove) || !removal.native_same_device || !removal.removed_once ||
      SUCCEEDED(removal.after_remove) || !removal.debug_clean || removal.completions != 1 ||
      !removal.completed_after_remove || removal.completed_result != std::numeric_limits<UINT64>::max() ||
      !removal.post_remove_checks || SUCCEEDED(removal.observed_removed))
    return dx12_fail(6, "native late-removal stimulus was not actually established");
  if (presented || presented.error().code == ErrorCode::frame_retry_required ||
      presented.error().code == ErrorCode::frame_not_ready || api.sets != 1 || api.gets != 1 ||
      api.polls != polls || api.closes != 1 || !api.close_ok)
    return dx12_fail(7, "late removed orphan reported success/retry, rebuilt or retained its removed epoch");
  if ((*orphan)->present() || (*orphan)->request_pixel_capture() || api.closes != 1)
    return dx12_fail(8, "late removed orphan regained finished-frame permission or double-closed pacing");
  // Keep the failed orphan alive: a fresh real device on this adapter proves that
  // native retirement happened in the post-submit failure branch, not its destructor.
  const auto retry = (*peer)->begin_frame();
  if (retry || retry.error().code != ErrorCode::frame_retry_required ||
      (*peer)->backend_info().device_generation != peer_before.device_generation + 1 ||
      (*peer)->backend_info().adapter_luid != peer_before.adapter_luid)
    return dx12_fail(9, "late orphan retirement blocked the live peer's own reconstruction");
  auto frame = wait_for_test_frame(**peer);
  if (!frame) return dx12_fail(10, "late-removal peer frame unavailable");
  (*frame)->clear({0, 1, 0, 1});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
      !dx12_pixel((*peer)->last_frame_pixels(), 32, 32, {0, 255, 0, 255}) ||
      !pacing_begin_debug_clean((*peer)->backend_info()))
    return dx12_fail(11, "late-removal live peer capture/debug failed");
  orphan->reset();
  if (api.closes != 1) return dx12_fail(12, "late removed orphan destructor double-closed native pacing");
  return 0;
}
