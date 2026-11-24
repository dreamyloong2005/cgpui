#include "dx12_device_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_adapter_option_internal.hpp"
#include "dx12_adapter_preference_internal.hpp"
#include <cstdlib>

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
} // namespace

Result<void> dx12_select_initial_adapter(Dx12State& state, Dx12InitializationFailureClass* failure) {
  if (failure) *failure = Dx12InitializationFailureClass::terminal;
  const char* variable = std::getenv("CGPUI_DX12_ADAPTER");
  const auto selection = dx12_parse_adapter_option(variable ? variable : "");
  if (!selection) return std::unexpected(selection.error());
  const bool automatic = selection->kind == Dx12AdapterKind::automatic;
  const bool preferred = selection->kind == Dx12AdapterKind::high_performance ||
                         selection->kind == Dx12AdapterKind::minimum_power;
  const bool indexed = selection->kind == Dx12AdapterKind::index;
  HRESULT result = E_FAIL;
  if (selection->kind == Dx12AdapterKind::warp) {
    result = state.factory->EnumWarpAdapter(IID_PPV_ARGS(&state.adapter));
    if (SUCCEEDED(result)) result = dx12_open_adapter_device(state);
  } else {
    for (UINT index = indexed ? selection->index : 0;; ++index) {
      state.adapter.Reset();
      result = preferred
          ? dx12_enumerate_preferred_adapter(*state.factory.Get(), selection->kind, index, state.adapter)
          : state.factory->EnumAdapters1(index, &state.adapter);
      if (FAILED(result)) break;
      DXGI_ADAPTER_DESC1 description{};
      result = state.adapter->GetDesc1(&description);
      if (FAILED(result)) break;
      if (!indexed && (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) continue;
      result = dx12_open_adapter_device(state);
      // A sealed recovery cohort is not evidence that another adapter should be
      // selected. Preserve the busy failure instead of silently changing LUID.
      if (SUCCEEDED(result) || indexed ||
          dx12_classify_initialization_failure(result) != Dx12InitializationFailureClass::backend_unavailable) break;
    }
  }
  if (FAILED(result)) {
    if (failure && automatic) *failure = dx12_classify_initialization_failure(result);
    return std::unexpected(dx12_error(result, "adapter/device selection"));
  }
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
} // namespace cgpui
