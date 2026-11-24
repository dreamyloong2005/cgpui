#pragma once

#include "dx12_native_internal.hpp"

namespace cgpui {
struct Dx12State;
Result<void> dx12_wait_idle(Dx12State& state);
struct Dx12SynchronizationWaitResult {
  DWORD status = WAIT_FAILED;
  DWORD error = ERROR_SUCCESS;
};

// Private native boundary. Tests may borrow a scripted implementation that
// outlives its state; normal renderers use the native implementation below.
class Dx12SynchronizationApi {
 public:
  virtual ~Dx12SynchronizationApi() = default;
  virtual HRESULT signal(ID3D12CommandQueue& queue, ID3D12Fence& fence,
                         UINT64 value) noexcept = 0;
  virtual UINT64 completed(ID3D12Fence& fence) noexcept = 0;
  virtual HRESULT event(ID3D12Fence& fence, UINT64 value, HANDLE handle) noexcept = 0;
  virtual Dx12SynchronizationWaitResult wait(HANDLE handle) noexcept = 0;
  virtual HRESULT removed(ID3D12Device& device) noexcept = 0;
  virtual void pause() noexcept = 0;
};

class Dx12NativeSynchronizationApi final : public Dx12SynchronizationApi {
 public:
  HRESULT signal(ID3D12CommandQueue& queue, ID3D12Fence& fence,
                 UINT64 value) noexcept override;
  UINT64 completed(ID3D12Fence& fence) noexcept override;
  HRESULT event(ID3D12Fence& fence, UINT64 value, HANDLE handle) noexcept override;
  Dx12SynchronizationWaitResult wait(HANDLE handle) noexcept override;
  HRESULT removed(ID3D12Device& device) noexcept override;
  void pause() noexcept override;
};

enum class Dx12SynchronizationProgress { pending, complete, removed };
struct Dx12SynchronizationProbe {
  Dx12SynchronizationProgress progress = Dx12SynchronizationProgress::pending;
  HRESULT error = S_OK;
  const char* operation = "pending fence";
};

// No allocations or waits: records one queue ticket and observes completion.
// Both foreground waits and noexcept destruction use the same evidence rules.
Dx12SynchronizationProbe dx12_poll_submission(
    Dx12State& state, Dx12SynchronizationApi& api) noexcept;
} // namespace cgpui
