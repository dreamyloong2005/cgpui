#include "dx12_state_internal.hpp"
#include "dx12_debug_internal.hpp"
#include <charconv>
#include <cstdlib>
#include <string_view>

namespace cgpui {
namespace {
std::string adapter_name(const wchar_t* text) {
  const int length = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
  if (length <= 1) return {};
  std::string result(static_cast<std::size_t>(length), '\0');
  WideCharToMultiByte(CP_UTF8, 0, text, -1, result.data(), length, nullptr, nullptr);
  result.pop_back();
  return result;
}
HRESULT try_device(Dx12State& state) {
  state.device.Reset();
  HRESULT result = D3D12CreateDevice(state.adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                    IID_PPV_ARGS(&state.device));
  if (FAILED(result)) return result;
  D3D12_FEATURE_DATA_SHADER_MODEL model{D3D_SHADER_MODEL_6_0};
  result = state.device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &model, sizeof(model));
  if (FAILED(result) || model.HighestShaderModel < D3D_SHADER_MODEL_6_0) {
    state.device.Reset();
    return DXGI_ERROR_UNSUPPORTED;
  }
  return S_OK;
}
Result<void> choose_adapter(Dx12State& state) {
  const char* variable = std::getenv("CGPUI_DX12_ADAPTER");
  const std::string_view selection = variable ? variable : "";
  HRESULT result = E_FAIL;
  if (selection == "warp") {
    result = state.factory->EnumWarpAdapter(IID_PPV_ARGS(&state.adapter));
    if (SUCCEEDED(result)) result = try_device(state);
  } else {
    UINT requested = 0;
    if (!selection.empty()) {
      const auto parsed = std::from_chars(selection.data(), selection.data() + selection.size(), requested);
      if (parsed.ec != std::errc{} || parsed.ptr != selection.data() + selection.size()) {
        return std::unexpected(Error{.code = ErrorCode::invalid_argument,
            .message = "CGPUI_DX12_ADAPTER must be warp or an adapter index"});
      }
    }
    for (UINT index = selection.empty() ? 0 : requested;; ++index) {
      state.adapter.Reset();
      result = state.factory->EnumAdapters1(index, &state.adapter);
      if (FAILED(result)) break;
      DXGI_ADAPTER_DESC1 description{};
      result = state.adapter->GetDesc1(&description);
      if (FAILED(result)) break;
      if (selection.empty() && (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) continue;
      result = try_device(state);
      if (SUCCEEDED(result) || !selection.empty()) break;
    }
  }
  if (FAILED(result)) return std::unexpected(dx12_error(result, "adapter/device selection"));
  DXGI_ADAPTER_DESC1 description{};
  result = state.adapter->GetDesc1(&description);
  if (FAILED(result)) return std::unexpected(dx12_error(result, "adapter description"));
  state.info.backend = RendererBackend::dx12;
  state.info.selection = RendererBackendSelection::dx12;
  state.info.adapter_name = adapter_name(description.Description);
  state.info.vendor_id = description.VendorId;
  state.info.adapter_luid = (std::uint64_t(std::uint32_t(description.AdapterLuid.HighPart)) << 32) |
                            description.AdapterLuid.LowPart;
  state.info.software_adapter = (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0;
  state.info.device_generation = 1;
  return {};
}
}

Result<void> dx12_create_device(Dx12State& state) {
  auto debug = dx12_enable_debug();
  if (!debug) return std::unexpected(debug.error());
  HRESULT result = CreateDXGIFactory2(*debug ? DXGI_CREATE_FACTORY_DEBUG : 0,
                                      IID_PPV_ARGS(&state.factory));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "factory creation"));
  if (auto adapter = choose_adapter(state); !adapter) return adapter;
  D3D12_COMMAND_QUEUE_DESC queue{};
  queue.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  result = state.device->CreateCommandQueue(&queue, IID_PPV_ARGS(&state.queue));
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
} // namespace cgpui
