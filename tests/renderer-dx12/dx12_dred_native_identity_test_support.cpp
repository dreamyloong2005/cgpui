#include "dx12_dred_native_test_support.hpp"
#include <algorithm>
#include <string_view>

namespace cgpui_test {
namespace {
bool request(const char* name) {
  const char* value = std::getenv(name);
  return value && std::string_view(value) == "1";
}
struct IncomingRequests { bool captured = false, debug = false, gpu = false; } incoming;
}
void dred_capture_requests() {
  incoming = {true, request("CGPUI_DX12_DEBUG"), request("CGPUI_DX12_GPU_VALIDATION")};
}
bool dred_select_debug(bool enabled) {
  dred_capture_requests();
  // Debug-on preserves root's GPU-validation request; debug-off intentionally overrides it.
  return _putenv_s("CGPUI_DX12_DEBUG", enabled ? "1" : "0") == 0 &&
      (enabled || _putenv_s("CGPUI_DX12_GPU_VALIDATION", "0") == 0);
}
bool dred_flush_output() {
  std::cout.flush();
  return std::cout.good();
}
bool dred_record_identity(const cgpui::RendererBackendInfo& info, const char* stage) {
  using namespace cgpui;
  const char* setting = std::getenv("CGPUI_DX12_ADAPTER");
  const std::string_view selector = setting ? setting : "";
  if ((!selector.empty() && selector != "warp" &&
      !std::all_of(selector.begin(), selector.end(), [](char c) { return c >= '0' && c <= '9'; })) ||
      info.backend != RendererBackend::dx12 || !info.adapter_luid || !*info.adapter_luid ||
      !info.device_generation || !info.fallback_reason.empty() ||
      (selector == "warp" && !info.software_adapter) || (selector.empty() && info.software_adapter)) return false;
  const auto device = dx12_acquire_native_device(info);
  if (!device || FAILED(device->GetDeviceRemovedReason())) return false;
  const auto native = device->GetAdapterLuid();
  const auto native_luid = (std::uint64_t(std::uint32_t(native.HighPart)) << 32) | native.LowPart;
  if (native_luid != *info.adapter_luid) return false;
  Microsoft::WRL::ComPtr<ID3D12InfoQueue> messages;
  const HRESULT queried = device.As(&messages);
  const bool debug = request("CGPUI_DX12_DEBUG"), gpu = request("CGPUI_DX12_GPU_VALIDATION");
  if ((debug || gpu) && !messages) return false;
  std::cout << std::dec << std::noboolalpha << "DRED_NATIVE_ID stage=" << stage << " requested_selector=" << (selector.empty() ? "default" : selector)
            << " actual_luid=" << *info.adapter_luid << " native_luid=" << native_luid
            << " actual_software=" << info.software_adapter << " generation=" << info.device_generation
            << " incoming_debug_request=" << (incoming.captured ? (incoming.debug ? "1" : "0") : "not_captured")
            << " incoming_gpu_request=" << (incoming.captured ? (incoming.gpu ? "1" : "0") : "not_captured")
            << " configured_debug_request=" << debug << " configured_gpu_request=" << gpu
            << " actual_infoqueue_received=" << bool(messages) << " infoqueue_hresult=" << std::hex << queried
            << std::dec << " gpu_validation_native_state=not_queried root_mapping_oracle_required=1\n";
  return dred_flush_output(); // Observation/output failure never returns a passing identity result.
}
} // namespace cgpui_test
