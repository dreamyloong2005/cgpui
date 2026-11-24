#include "dx12_owner_recovery_test_support.hpp"
#include "../support/frame_wait_test_support.hpp"
#include <functional>

namespace cgpui_test {
using namespace cgpui;
namespace {
bool await(const std::function<bool()>& condition) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!condition() && std::chrono::steady_clock::now() < deadline) {
    owner_recovery_pump();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  return condition();
}
template <typename T> bool pending(const Result<T>& result) {
  return !result && result.error().code == ErrorCode::renderer_recovery_pending;
}
} // namespace

int run_owner_recovery_case(OwnerRecoveryCase mode) {
  Dx12RecoveryDeadline watchdog;
  ManagedRecoveryWindow origin;
  if (!watchdog.done || !origin.renderer || !owner_recovery_paint(*origin.renderer, true))
    return dx12_fail(1, "managed origin fixture failed");
  const auto before = origin.renderer->backend_info();
  const auto extent = origin.window->state();
  const auto capture_width = origin.renderer->last_frame_pixels()->width;
  const auto capture_height = origin.renderer->last_frame_pixels()->height;
  const Size desired{extent.framebuffer_size.width + 8, extent.framebuffer_size.height + 8};
  std::unique_ptr<RenderFrame> interrupted;
  if (mode == OwnerRecoveryCase::pumping) {
    auto frame = wait_for_test_frame(*origin.renderer, {.message_owner = std::this_thread::get_id()});
    if (!frame) return dx12_fail(17, "origin present admission failed");
    interrupted = std::move(*frame);
  }
  OwnerRecoveryPeer peer(mode);
  peer.ready.wait();
  if (!peer.prepared || peer.info.adapter_luid != before.adapter_luid)
    return dx12_fail(2, "managed foreign peer preparation/adapter failed");
  if (!dx12_remove_native_device(before)) return dx12_fail(3, "actual native removal failed");
  const auto started = std::chrono::steady_clock::now();
  if (interrupted) {
    if (!pending(interrupted->present())) return dx12_fail(4, "present did not expose unfinished owner retirement");
    const auto repeated = interrupted->present();
    if (repeated || repeated.error().code != ErrorCode::invalid_argument)
      return dx12_fail(18, "pending present did not consume the old frame");
  } else if (mode == OwnerRecoveryCase::busy) {
    if (!pending(origin.renderer->resize(desired, extent.scale)))
      return dx12_fail(4, "resize did not expose unfinished owner retirement");
  } else if (!pending(origin.renderer->begin_frame()))
    return dx12_fail(4, "begin did not expose unfinished owner retirement");
  if (origin.renderer->backend_info().device_generation != before.device_generation ||
      origin.renderer->last_frame_pixels()) return dx12_fail(5, "pending recovery published success or capture");
  if (mode == OwnerRecoveryCase::parked) {
    bool timed_out = false;
    while (std::chrono::steady_clock::now() - started < std::chrono::seconds(7)) {
      auto next = origin.renderer->begin_frame();
      if (next) return dx12_fail(6, "parked owner unexpectedly permitted a frame");
      if (next.error().code != ErrorCode::renderer_recovery_pending) {
        const auto elapsed = std::chrono::steady_clock::now() - started;
        if (next.error().code != ErrorCode::renderer_initialization_failed || elapsed < std::chrono::seconds(4))
          return dx12_fail(7, "parked owner failed before bounded pending deadline");
        timed_out = true;
        break;
      }
      owner_recovery_pump();
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    if (!timed_out || origin.renderer->backend_info().device_generation != before.device_generation)
      return dx12_fail(8, "parked owner retries extended deadline or generation");
    if (peer.finish()) return dx12_fail(9, "parked peer was purged off-owner or mutated before destructor");
  } else {
    peer.pump.store(true);
    if (mode == OwnerRecoveryCase::busy) {
      if (!await([&] { return peer.busy_seen.load(); }) || !pending(origin.renderer->begin_frame()))
        return dx12_fail(10, "busy callback acknowledged or failed before outer operation exit");
      peer.release_busy.store(true);
    }
    if (!await([&] { return peer.retired.load(); })) return dx12_fail(11, "owner dispatch did not retire native peer");
  }
  auto rebuilt = origin.renderer->begin_frame();
  if (rebuilt || rebuilt.error().code != ErrorCode::frame_retry_required)
    return dx12_fail(12, "completed owner retirement did not rebuild origin");
  const auto after = origin.renderer->backend_info();
  if (after.adapter_luid != before.adapter_luid || after.device_generation != before.device_generation + 1 ||
      !owner_recovery_paint(*origin.renderer, false)) return dx12_fail(13, "rebuild changed identity or lost retained image");
  if (mode == OwnerRecoveryCase::busy) {
    const auto* capture = origin.renderer->last_frame_pixels();
    if (capture->width != capture_width || capture->height != capture_height)
      return dx12_fail(19, "pending resize falsely applied the requested extent");
    if (!origin.renderer->resize(desired, extent.scale) || !owner_recovery_paint(*origin.renderer, false) ||
        origin.renderer->last_frame_pixels()->width != capture_width + 8 ||
        origin.renderer->last_frame_pixels()->height != capture_height + 8)
      return dx12_fail(20, "explicit latest resize did not apply after recovery");
  }
  peer.origin_rebuilt.store(true);
  if (mode != OwnerRecoveryCase::parked) {
    // finish must not stop the peer before its lazy rebuild and pixels assertion.
    if (!await([&] { return peer.completed.load(); })) return dx12_fail(14, "peer lazy rebuild did not finish");
    if (peer.finish()) return dx12_fail(15, "peer native/capture/generation checks failed");
  }
  return dx12_recovered_native_device_clean(after) ? 0 : dx12_fail(16, "new native generation debug errors");
}
} // namespace cgpui_test
