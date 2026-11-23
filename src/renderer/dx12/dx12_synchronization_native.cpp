#include "dx12_synchronization_internal.hpp"

namespace cgpui {
HRESULT Dx12NativeSynchronizationApi::signal(
    ID3D12CommandQueue& queue, ID3D12Fence& fence, UINT64 value) noexcept {
  return queue.Signal(&fence, value);
}
UINT64 Dx12NativeSynchronizationApi::completed(ID3D12Fence& fence) noexcept {
  return fence.GetCompletedValue();
}
HRESULT Dx12NativeSynchronizationApi::event(
    ID3D12Fence& fence, UINT64 value, HANDLE handle) noexcept {
  return fence.SetEventOnCompletion(value, handle);
}
Dx12SynchronizationWaitResult Dx12NativeSynchronizationApi::wait(HANDLE handle) noexcept {
  const DWORD status = WaitForSingleObject(handle, INFINITE);
  return {.status = status, .error = status == WAIT_FAILED ? GetLastError() : ERROR_SUCCESS};
}
HRESULT Dx12NativeSynchronizationApi::removed(ID3D12Device& device) noexcept {
  return device.GetDeviceRemovedReason();
}
void Dx12NativeSynchronizationApi::pause() noexcept { Sleep(1); }
} // namespace cgpui
