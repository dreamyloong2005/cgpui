#include "dx12_orphan_native_removal_test_support.hpp"
#include "dx12_recovery_test_support.hpp"

namespace cgpui_test {
HRESULT Dx12OrphanNativeRemoval::signal(
    ID3D12CommandQueue& queue, ID3D12Fence& fence, UINT64 value) noexcept {
  ++signals;
  const HRESULT result = native_.signal(queue, fence, value);
  if (signals != 1) return result;
  signal_result = result;
  if (FAILED(result)) return result;
  Microsoft::WRL::ComPtr<ID3D12Device5> device;
  Microsoft::WRL::ComPtr<ID3D12InfoQueue> messages;
  Microsoft::WRL::ComPtr<IUnknown> queue_device, fence_device;
  if (FAILED(queue.GetDevice(IID_PPV_ARGS(&device))) || !device ||
      FAILED(device.As(&messages)) || !messages) return result;
  if (FAILED(device.As(&queue_device)) || FAILED(fence.GetDevice(IID_PPV_ARGS(&fence_device))) ||
      queue_device.Get() != fence_device.Get()) return result;
  native_same_device = true;
  before_remove = native_.removed(*device.Get());
  if (FAILED(before_remove) || !dx12_recovery_debug_messages_safe(messages.Get())) return result;
  ++removal_calls;
  device->RemoveDevice(); // Actual native fault, after actual native Signal, not a fabricated HRESULT.
  after_remove = native_.removed(*device.Get());
  removed_once = FAILED(after_remove);
  debug_clean = dx12_recovery_debug_messages_safe(messages.Get(), true);
  return result; // The genuine Signal result; subsequent native fence/device observations drive failure.
}
UINT64 Dx12OrphanNativeRemoval::completed(ID3D12Fence& fence) noexcept {
  ++completions;
  completed_after_remove = removed_once;
  completed_result = native_.completed(fence);
  return completed_result; // Never substitute readiness or the native removal sentinel.
}
HRESULT Dx12OrphanNativeRemoval::event(ID3D12Fence& fence, UINT64 value, HANDLE handle) noexcept {
  return native_.event(fence, value, handle);
}
cgpui::Dx12SynchronizationWaitResult Dx12OrphanNativeRemoval::wait(HANDLE handle) noexcept {
  return native_.wait(handle);
}
HRESULT Dx12OrphanNativeRemoval::removed(ID3D12Device& device) noexcept {
  const HRESULT result = native_.removed(device);
  if (removed_once) { ++post_remove_checks; observed_removed = result; }
  return result;
}
void Dx12OrphanNativeRemoval::pause() noexcept { native_.pause(); }
} // namespace cgpui_test
