#pragma once

#include "../../src/renderer/dx12/dx12_state_internal.hpp"
#include "../../src/renderer/dx12/dx12_synchronization_internal.hpp"
#include <iostream>
#include <memory>

namespace cgpui_test {
// Real device objects satisfy the production ownership boundary, but these
// tests never ExecuteCommandLists or submit GPU work. Only sync results vary.
struct Dx12SyncScript : cgpui::Dx12SynchronizationApi {
  unsigned signal_calls = 0;
  UINT64 last_signal = 0;
  UINT64 completion = 0;
  HRESULT event_result = E_ACCESSDENIED;
  cgpui::Dx12SynchronizationWaitResult wait_result{WAIT_FAILED, ERROR_ACCESS_DENIED};

  HRESULT signal(ID3D12CommandQueue&, ID3D12Fence&, UINT64 value) noexcept override {
    ++signal_calls;
    last_signal = value;
    return S_OK;
  }
  UINT64 completed(ID3D12Fence&) noexcept override { return completion; }
  HRESULT event(ID3D12Fence&, UINT64, HANDLE) noexcept override { return event_result; }
  cgpui::Dx12SynchronizationWaitResult wait(HANDLE) noexcept override { return wait_result; }
  HRESULT removed(ID3D12Device&) noexcept override { return S_OK; }
  void pause() noexcept override {}
};

inline std::unique_ptr<cgpui::Dx12State> dx12_sync_state(cgpui::Dx12SynchronizationApi& api) {
  auto state = std::make_unique<cgpui::Dx12State>();
  if (const auto created = cgpui::dx12_create_device(*state); !created) {
    std::cerr << "DX12 synchronization setup: " << created.error().message << '\n';
    return {};
  }
  state->synchronization_api = &api;
  state->submitted = true; // Synthetic pending work; no native GPU commands exist.
  return state;
}

inline int dx12_sync_fail(int code, const char* message) {
  std::cerr << "DX12 synchronization: " << message << '\n';
  return code;
}
} // namespace cgpui_test
