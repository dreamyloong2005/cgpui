#include "dx12_device_internal.hpp"
#include "dx12_state_internal.hpp"
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
} // namespace

Result<void> dx12_select_initial_adapter(Dx12State& state) {
  const char* variable = std::getenv("CGPUI_DX12_ADAPTER");
  const std::string_view selection = variable ? variable : "";
  HRESULT result = E_FAIL;
  if (selection == "warp") {
    result = state.factory->EnumWarpAdapter(IID_PPV_ARGS(&state.adapter));
    if (SUCCEEDED(result)) result = dx12_open_adapter_device(state);
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
      result = dx12_open_adapter_device(state);
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
} // namespace cgpui
