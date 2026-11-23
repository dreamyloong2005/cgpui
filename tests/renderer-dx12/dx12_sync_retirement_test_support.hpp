#pragma once

#include "dx12_sync_test_support.hpp"
#include "dx12_sync_resource_sentinel.hpp"
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <thread>

namespace cgpui_test {
struct Dx12ControlledRetirement final : Dx12SyncScript {
  std::mutex mutex;
  std::condition_variable changed;
  bool fail_signal_until_completion = false;
  bool paused = false;
  bool permit_completion = false;
  bool destroyed = false;

  HRESULT signal(ID3D12CommandQueue&, ID3D12Fence&, UINT64 value) noexcept override {
    std::lock_guard lock(mutex);
    ++signal_calls;
    last_signal = value;
    return fail_signal_until_completion && !permit_completion ? E_ACCESSDENIED : S_OK;
  }
  UINT64 completed(ID3D12Fence&) noexcept override {
    std::lock_guard lock(mutex);
    return permit_completion ? last_signal : 0;
  }
  void pause() noexcept override {
    std::unique_lock lock(mutex);
    paused = true;
    changed.notify_all();
    changed.wait(lock, [&] { return permit_completion; });
  }
};

inline bool dx12_retirement_retains_until_complete(
    std::unique_ptr<cgpui::Dx12State> state, Dx12ControlledRetirement& script,
    const std::atomic_bool& released) {
  std::thread destroyer([owned = std::move(state), &script]() mutable {
    owned.reset(); // Actual state destructor, not a policy-only simulation.
    std::lock_guard lock(script.mutex);
    script.destroyed = true;
    script.changed.notify_all();
  });
  std::unique_lock lock(script.mutex);
  const bool reached = script.changed.wait_for(lock, std::chrono::seconds(5), [&] {
    return script.paused || script.destroyed;
  });
  const bool retained = reached && script.paused && !script.destroyed &&
      !released.load(std::memory_order_acquire);
  script.permit_completion = true;
  script.changed.notify_all();
  const bool finished = script.changed.wait_for(lock, std::chrono::seconds(5), [&] {
    return script.destroyed;
  });
  lock.unlock();
  if (!finished) {
    dx12_sync_fail(3, "retirement did not finish after completion was permitted");
    std::abort(); // Test watchdog, never a production resource-release policy.
  }
  destroyer.join();
  return retained && released.load(std::memory_order_acquire);
}
} // namespace cgpui_test
