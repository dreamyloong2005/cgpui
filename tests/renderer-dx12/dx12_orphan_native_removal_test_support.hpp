#pragma once

#include "../../src/renderer/dx12/dx12_synchronization_internal.hpp"

namespace cgpui_test {
// Forwards all synchronization observations, then removes the real queue device
// after its first successful post-Present Signal. Holds no persistent COM owners.
class Dx12OrphanNativeRemoval final : public cgpui::Dx12SynchronizationApi {
 public:
  unsigned signals = 0, removal_calls = 0, completions = 0, post_remove_checks = 0;
  HRESULT signal_result = E_FAIL, before_remove = E_FAIL, after_remove = S_OK;
  HRESULT observed_removed = S_OK;
  UINT64 completed_result = 0;
  bool removed_once = false, debug_clean = false, native_same_device = false, completed_after_remove = false;
  HRESULT signal(ID3D12CommandQueue& queue, ID3D12Fence& fence, UINT64 value) noexcept override;
  UINT64 completed(ID3D12Fence& fence) noexcept override;
  HRESULT event(ID3D12Fence& fence, UINT64 value, HANDLE handle) noexcept override;
  cgpui::Dx12SynchronizationWaitResult wait(HANDLE handle) noexcept override;
  HRESULT removed(ID3D12Device& device) noexcept override;
  void pause() noexcept override;
 private:
  cgpui::Dx12NativeSynchronizationApi native_;
};
} // namespace cgpui_test
