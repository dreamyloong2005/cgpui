#include "../support/frame_wait_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "dx12_diagnostics_support.hpp"

#include <optional>
#include <string_view>
#include <vector>

using namespace cgpui;
using namespace cgpui_test;

namespace {
constexpr const char marker[] = "CGPUI capture publication late-error 20261124";
bool only_injected_error(ID3D12InfoQueue& messages) {
  unsigned injected = 0;
  for (UINT64 index = 0; index < messages.GetNumStoredMessagesAllowedByRetrievalFilter(); ++index) {
    SIZE_T length = 0;
    if (FAILED(messages.GetMessage(index, nullptr, &length))) return false;
    std::vector<unsigned char> storage(length);
    auto* message = reinterpret_cast<D3D12_MESSAGE*>(storage.data());
    if (FAILED(messages.GetMessage(index, message, &length))) return false;
    if (message->Severity != D3D12_MESSAGE_SEVERITY_ERROR &&
        message->Severity != D3D12_MESSAGE_SEVERITY_CORRUPTION) continue;
    if (message->Severity != D3D12_MESSAGE_SEVERITY_ERROR ||
        message->ID != D3D12_MESSAGE_ID_STRING_FROM_APPLICATION || !message->pDescription ||
        std::string_view(message->pDescription) != marker) {
      std::cerr << "unexpected capture debug message " << message->ID << '\n';
      return false;
    }
    ++injected;
  }
  return injected == 1;
}

int run_case(bool warm) {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "capture publication HWND unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  const auto info = (*renderer)->backend_info();
  if (info.backend != RendererBackend::dx12 || !info.adapter_luid)
    return dx12_fail(3, "capture publication must exercise actual DX12");
  if ((*renderer)->last_frame_pixels() || (*renderer)->last_frame_diagnostic_snapshot())
    return dx12_fail(4, "fresh renderer already published frame state");

  std::optional<RendererFrameDiagnosticSnapshot> saved_diagnostics;
  const RendererFrameDiagnosticSnapshot* borrowed_diagnostics = nullptr;
  if (warm) {
    auto frame = wait_for_test_frame(**renderer);
    if (!frame) return dx12_fail(5, frame.error().message.c_str());
    (*frame)->clear({0, 0, 1, 1});
    (*frame)->draw_rect({.rect = {{4, 4}, {12, 12}}, .color = {1, 0, 0, 1}});
    if (!(*frame)->request_pixel_capture()) return dx12_fail(6, "warm capture request failed");
    const auto presented = (*frame)->present(); // No Present retry masking.
    if (!presented) return dx12_fail(7, presented.error().message.c_str());
    if (!dx12_pixel((*renderer)->last_frame_pixels(), 8, 8, {255, 0, 0, 255}) ||
        !dx12_pixel((*renderer)->last_frame_pixels(), 32, 32, {0, 0, 255, 255}))
      return dx12_fail(8, "warm capture pixels differ");
    borrowed_diagnostics = (*renderer)->last_frame_diagnostic_snapshot();
    if (!borrowed_diagnostics) return dx12_fail(9, "warm diagnostics absent");
    saved_diagnostics = *borrowed_diagnostics;
    // Do not retain/dereference the old borrowed pixel pointer across Present.
  }

  auto frame = wait_for_test_frame(**renderer);
  if (!frame) return dx12_fail(10, frame.error().message.c_str());
  (*frame)->clear({0, 1, 0, 1});
  (*frame)->draw_rect({.rect = {{0, 0}, {8, 8}}, .color = {1, 1, 0, 1}});
  (*frame)->draw_rect({.rect = {{16, 0}, {8, 8}}, .color = {1, 0, 1, 1}});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(11, "candidate capture request failed");

  // Existing test helper resolves the real process/adapter D3D12 singleton.
  // Keep it live: this test must NOT remove or rebuild the native device.
  auto native = dx12_acquire_native_device(info);
  Microsoft::WRL::ComPtr<ID3D12InfoQueue> messages;
  if (!native || FAILED(native->GetDeviceRemovedReason()) ||
      FAILED(native.As(&messages)) || !messages)
    return dx12_fail(12, "live debug-layer device unavailable");
  if (!dx12_recovery_debug_messages_safe(messages.Get()))
    return dx12_fail(13, "unexpected debug error before controlled injection");
  messages->ClearStoredMessages();

  // Inject only after successful begin/request. Native draw, fence and readback
  // still execute; the current production path observes this at its late check.
  if (FAILED(messages->AddApplicationMessage(D3D12_MESSAGE_SEVERITY_ERROR, marker)))
    return dx12_fail(14, "could not add the controlled native debug message");
  const auto rejected = (*frame)->present(); // Raw result, never a retry helper.
  const bool expected_messages = only_injected_error(*messages.Get());
  messages->ClearStoredMessages(); // Do not contaminate teardown/other fixtures.
  if (!expected_messages) return dx12_fail(21, "native queue contained errors besides the controlled marker");
  if (rejected || rejected.error().code != ErrorCode::renderer_initialization_failed ||
      std::string_view(rejected.error().message).find(marker) == std::string_view::npos)
    return dx12_fail(15, "Present did not fail at the controlled late debug check");
  if (FAILED(native->GetDeviceRemovedReason()) ||
      (*renderer)->backend_info().device_generation != info.device_generation)
    return dx12_fail(16, "late debug error unexpectedly became device recovery");
  const auto* diagnostics = (*renderer)->last_frame_diagnostic_snapshot();
  if (warm ? diagnostics != borrowed_diagnostics || !diagnostic_same(diagnostics, *saved_diagnostics)
           : diagnostics != nullptr)
    return dx12_fail(17, "rejected frame replaced successful diagnostic history");
  if ((*renderer)->last_frame_pixels() != nullptr)
    return dx12_fail(18, "rejected late-error frame exposed candidate capture pixels");
  return 0;
}
} // namespace

int main() {
  // Standalone test process; set once before ANY native device is constructed.
  // Existing adapter and GPU-validation environment selections remain intact.
  if (_putenv_s("CGPUI_DX12_DEBUG", "1") != 0)
    return dx12_fail(19, "could not require the native debug layer");
  Dx12RecoveryDeadline deadline;
  if (!deadline.done) return dx12_fail(20, "test deadline unavailable");
  int result = 0;
  for (const bool warm : {false, true}) {
    const int observed = run_case(warm);
    std::cout << "CAPTURE_PUBLICATION warm=" << warm << " result=" << observed << '\n';
    if (observed && !result) result = observed;
  }
  return result;
}
