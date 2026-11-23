#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_device_cohort_internal.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"
#include <latch>

using namespace cgpui;
using namespace cgpui_test;

namespace {
std::shared_ptr<Dx12State> participant(const Dx12Window& window) {
  auto state = std::make_shared<Dx12State>();
  std::lock_guard gate(state->operation_mutex);
  state->hwnd = window.hwnd;
  state->width = state->height = 64;
  state->scale = {1};
  if (!dx12_create_device(*state) || !dx12_create_pipeline(*state) ||
      !dx12_create_swapchain(*state) || !dx12_check_debug_errors(*state)) return {};
  dx12_register_device_cohort(state);
  return state;
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done) return dx12_fail(1, "busy-cohort watchdog unavailable");
  Dx12Window first_window, second_window;
  if (!first_window.hwnd || !second_window.hwnd) return dx12_fail(2, "busy-cohort windows unavailable");
  auto origin = participant(first_window), peer = participant(second_window);
  if (!origin || !peer) return dx12_fail(3, "busy-cohort native initialization failed");
  if (origin->info.adapter_luid != peer->info.adapter_luid ||
      origin->window_thread_id != GetCurrentThreadId() ||
      peer->window_thread_id != GetCurrentThreadId()) {
    return dx12_fail(4, "busy-cohort participants do not share adapter and HWND owner");
  }
  const auto generation = peer->info.device_generation;
  auto* const device = peer->device.Get();
  auto* const queue = peer->queue.Get();
  auto* const fence = peer->fence.Get();
  auto* const swapchain = peer->swapchain.Get();
  peer->frame_active = true; // Native retirement must not cancel a CPU frame.
  if (!dx12_remove_native_device(origin->info)) return dx12_fail(5, "busy-cohort removal unverified");

  std::latch held(1), release(1);
  std::thread holder([&] {
    std::lock_guard gate(peer->operation_mutex);
    held.count_down();
    release.wait();
  });
  held.wait();
  {
    std::lock_guard gate(origin->operation_mutex);
    // A blocking peer lock deadlocks here and is caught by the process watchdog;
    // successful return therefore proves the nonblocking try-lock behavior.
    dx12_retire_removed_cohort_peers(*origin);
  }
  release.count_down();
  holder.join();

  std::lock_guard origin_gate(origin->operation_mutex);
  std::lock_guard peer_gate(peer->operation_mutex);
  // Only inspect peer fields after its foreign gate owner has exited.
  if (peer->recovery_pending || peer->failed || peer->device.Get() != device ||
      peer->queue.Get() != queue || peer->fence.Get() != fence ||
      peer->swapchain.Get() != swapchain) {
    return dx12_fail(6, "busy peer was mutated or released instead of skipped");
  }
  dx12_retire_removed_cohort_peers(*origin);
  if (!peer->recovery_pending || !peer->failed || peer->device || peer->queue ||
      peer->fence || peer->swapchain || peer->submitted || !peer->frame_active ||
      peer->info.device_generation != generation) {
    return dx12_fail(7, "released peer gate did not allow native-only retirement");
  }
  return 0;
}
