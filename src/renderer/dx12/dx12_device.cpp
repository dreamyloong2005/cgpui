#include "dx12_state_internal.hpp"
#include "dx12_debug_internal.hpp"
#include "dx12_device_internal.hpp"
#include "dx12_dred_api_internal.hpp"

namespace cgpui {
namespace {
Result<void> create_factory(Dx12State& state, Dx12InitializationFailureClass* failure = nullptr) {
  auto debug = dx12_enable_debug();
  if (!debug) return std::unexpected(debug.error());
  state.dred_status.configure_setup(dx12_dred_setup(*debug));
  const HRESULT result = CreateDXGIFactory2(*debug ? DXGI_CREATE_FACTORY_DEBUG : 0,
                                            IID_PPV_ARGS(&state.factory));
  if (FAILED(result)) {
    if (failure) *failure = dx12_classify_initialization_failure(result);
    return std::unexpected(dx12_error(result, "factory creation"));
  }
  return {};
}

Result<void> create_queue_objects(Dx12State& state) {
  D3D12_COMMAND_QUEUE_DESC queue{};
  queue.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  HRESULT result = state.device->CreateCommandQueue(&queue, IID_PPV_ARGS(&state.queue));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "command queue"));
  result = state.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&state.fence));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "fence"));
  state.fence_event.value = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  if (!state.fence_event.value) return std::unexpected(dx12_error(HRESULT_FROM_WIN32(GetLastError()), "fence event"));
  result = state.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                               IID_PPV_ARGS(&state.allocator));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "command allocator"));
  result = state.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
      state.allocator.Get(), nullptr, IID_PPV_ARGS(&state.commands));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "command list"));
  result = state.commands->Close();
  if (FAILED(result)) return std::unexpected(dx12_error(result, "initial command list close"));
  return {};
}
} // namespace

Result<void> dx12_create_device(Dx12State& state, Dx12InitializationFailureClass* failure) {
  if (failure) *failure = Dx12InitializationFailureClass::terminal;
  if (auto factory = create_factory(state, failure); !factory) return factory;
  if (auto adapter = dx12_select_initial_adapter(state, failure); !adapter) return adapter;
  return create_queue_objects(state);
}

Result<void> dx12_create_device_on_adapter(Dx12State& state, std::uint64_t luid) {
  if (auto factory = create_factory(state); !factory) return factory;
  if (auto adapter = dx12_select_pinned_adapter(state, luid); !adapter) return adapter;
  return create_queue_objects(state);
}
} // namespace cgpui
