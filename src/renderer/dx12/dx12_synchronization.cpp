#include "dx12_state_internal.hpp"
#include <limits>

namespace cgpui {
Result<void> dx12_wait_idle(Dx12State& state) {
  if (!state.submitted) return {};
  const auto value = ++state.fence_value;
  HRESULT result = state.queue->Signal(state.fence.Get(), value);
  if (FAILED(result)) return std::unexpected(dx12_error(result, "queue signal"));
  if (state.fence->GetCompletedValue() < value) {
    result = state.fence->SetEventOnCompletion(value, state.fence_event.value);
    if (FAILED(result)) return std::unexpected(dx12_error(result, "fence event registration"));
    // Never recycle uploads/allocator/targets before the GPU is finished.
    const DWORD waited = WaitForSingleObject(state.fence_event.value, INFINITE);
    if (waited != WAIT_OBJECT_0) {
      return std::unexpected(dx12_error(HRESULT_FROM_WIN32(GetLastError()), "fence wait"));
    }
  }
  if (state.fence->GetCompletedValue() == std::numeric_limits<UINT64>::max()) {
    return std::unexpected(dx12_error(state.device->GetDeviceRemovedReason(), "device removed"));
  }
  state.submitted = false;
  return {};
}

Dx12State::~Dx12State() {
  if (submitted && queue && fence && fence_event.value) (void)dx12_wait_idle(*this);
}

void dx12_transition(ID3D12GraphicsCommandList& commands, ID3D12Resource& resource,
                     D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
  D3D12_RESOURCE_BARRIER barrier{};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Transition.pResource = &resource;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  barrier.Transition.StateBefore = before;
  barrier.Transition.StateAfter = after;
  commands.ResourceBarrier(1, &barrier);
}
} // namespace cgpui
