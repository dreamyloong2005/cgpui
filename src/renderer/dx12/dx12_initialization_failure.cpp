#include "dx12_initialization_failure_internal.hpp"
#include "dx12_adapter_option_internal.hpp"
#include <cstdlib>
#include <string_view>

namespace cgpui {
Dx12InitializationFailureClass dx12_classify_initialization_failure(HRESULT result) noexcept {
  // Busy/recovery, lost devices, invalid arguments and resource exhaustion are
  // not device-capability evidence, even when native APIs wrap them as init errors.
  return result == DXGI_ERROR_UNSUPPORTED || result == DXGI_ERROR_NOT_FOUND ||
      result == E_NOINTERFACE || result == E_NOTIMPL
      ? Dx12InitializationFailureClass::backend_unavailable : Dx12InitializationFailureClass::terminal;
}
Error dx12_initialization_error(HRESULT result, const char* operation, Dx12InitializationFailureClass* failure) {
  if (failure) *failure = dx12_classify_initialization_failure(result);
  return dx12_error(result, operation);
}
Result<bool> dx12_initialization_fallback_allowed() {
  bool allowed = true;
  for (const char* name : {"CGPUI_DX12_DEBUG", "CGPUI_DX12_GPU_VALIDATION"}) {
    const char* value = std::getenv(name);
    const std::string_view setting = value ? value : "";
    if (!setting.empty() && setting != "0" && setting != "1")
      return std::unexpected(Error{ErrorCode::invalid_argument, std::string(name) + " must be 0 or 1"});
    if (setting == "1") allowed = false;
  }
  const char* value = std::getenv("CGPUI_DX12_ADAPTER");
  const auto adapter = dx12_parse_adapter_option(value ? value : "");
  if (!adapter) return std::unexpected(adapter.error());
  return allowed && adapter->kind == Dx12AdapterKind::automatic;
}
} // namespace cgpui
