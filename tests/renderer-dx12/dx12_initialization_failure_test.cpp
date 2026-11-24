#include "../../src/renderer/dx12/dx12_initialization_failure_internal.hpp"
#include <cstdlib>
#include <string>
#include <array>

using namespace cgpui;
namespace {
struct Environment {
  const char* name;
  std::string previous;
  explicit Environment(const char* key) : name(key) {
    if (const char* value = std::getenv(key)) previous = value;
    _putenv_s(key, "");
  }
  ~Environment() { _putenv_s(name, previous.c_str()); }
};
}
int main() {
  Environment adapter("CGPUI_DX12_ADAPTER"), debug("CGPUI_DX12_DEBUG"), gpu("CGPUI_DX12_GPU_VALIDATION");
  if (!dx12_initialization_fallback_allowed().value_or(false)) return 1;
  for (const char* name : {debug.name, gpu.name, adapter.name}) {
    _putenv_s(name, "invalid");
    auto invalid = dx12_initialization_fallback_allowed();
    if (invalid || invalid.error().code != ErrorCode::invalid_argument) return 2;
    _putenv_s(name, "1");
    auto pinned = dx12_initialization_fallback_allowed();
    if (!pinned || *pinned) return 3;
    _putenv_s(name, "");
  }
  for (HRESULT status : {DXGI_ERROR_DEVICE_REMOVED, DXGI_ERROR_DEVICE_RESET, DXGI_ERROR_DEVICE_HUNG,
       HRESULT_FROM_WIN32(ERROR_BUSY), E_INVALIDARG, E_OUTOFMEMORY, E_FAIL})
    if (dx12_classify_initialization_failure(status) != Dx12InitializationFailureClass::terminal) return 4;
  for (HRESULT status : {DXGI_ERROR_UNSUPPORTED, DXGI_ERROR_NOT_FOUND, E_NOTIMPL, E_NOINTERFACE})
    if (dx12_classify_initialization_failure(status) != Dx12InitializationFailureClass::backend_unavailable) return 5;
  return 0;
}
