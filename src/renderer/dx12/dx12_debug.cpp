#include "dx12_debug_internal.hpp"
#include "dx12_state_internal.hpp"
#include <d3d12sdklayers.h>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <optional>
#include <vector>

namespace cgpui {
namespace {
Result<bool> configure_debug(bool enabled, bool gpu_validation) {
  if (!enabled) return false;
  Dx12Ptr<ID3D12Debug> debug;
  const HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "debug layer"));
  debug->EnableDebugLayer();
  if (gpu_validation) {
    Dx12Ptr<ID3D12Debug1> validation;
    const HRESULT queried = debug.As(&validation);
    if (FAILED(queried)) return std::unexpected(dx12_error(queried, "GPU validation"));
    validation->SetEnableGPUBasedValidation(TRUE);
  }
  return true;
}
} // namespace

Result<bool> dx12_enable_debug() {
  const char* setting = std::getenv("CGPUI_DX12_DEBUG");
  const char* gpu = std::getenv("CGPUI_DX12_GPU_VALIDATION");
  const bool gpu_validation = gpu && std::strcmp(gpu, "1") == 0;
  const bool enabled = gpu_validation || (setting && std::strcmp(setting, "1") == 0);
  // These D3D12 switches are process-wide and must precede device creation.
  // Multiple windows reuse the first configuration, never toggle live devices.
  static std::mutex mutex;
  static std::optional<Result<bool>> configured;
  static bool original_enabled = false, original_gpu_validation = false;
  std::lock_guard lock(mutex);
  if (configured) {
    if (enabled != original_enabled || gpu_validation != original_gpu_validation) {
      return std::unexpected(Error{.code = ErrorCode::invalid_argument,
          .message = "DX12 debug configuration cannot change after initial device setup"});
    }
    return *configured;
  }
  original_enabled = enabled;
  original_gpu_validation = gpu_validation;
  configured = configure_debug(enabled, gpu_validation);
  return *configured;
}

Result<void> dx12_check_debug_errors(Dx12State& state) {
  Dx12Ptr<ID3D12InfoQueue> messages;
  if (FAILED(state.device.As(&messages))) return {};
  const auto count = messages->GetNumStoredMessagesAllowedByRetrievalFilter();
  for (UINT64 index = 0; index != count; ++index) {
    SIZE_T length = 0;
    if (FAILED(messages->GetMessage(index, nullptr, &length))) continue;
    std::vector<unsigned char> storage(length);
    auto* message = reinterpret_cast<D3D12_MESSAGE*>(storage.data());
    if (FAILED(messages->GetMessage(index, message, &length))) continue;
    if (message->Severity == D3D12_MESSAGE_SEVERITY_ERROR ||
        message->Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION) {
      return std::unexpected(Error{.code = ErrorCode::renderer_initialization_failed,
          .message = std::string("DX12 debug layer: ") + message->pDescription});
    }
  }
  messages->ClearStoredMessages();
  return {};
}
} // namespace cgpui
