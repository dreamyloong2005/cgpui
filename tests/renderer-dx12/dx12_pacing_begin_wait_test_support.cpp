#include "dx12_pacing_begin_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include <chrono>

namespace cgpui_test {
bool pacing_begin_debug_enable() {
  cgpui::Dx12Ptr<ID3D12Debug> debug;
  if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) return false;
  debug->EnableDebugLayer();
  return true;
}
bool pacing_begin_debug_clean(const cgpui::RendererBackendInfo& info) {
  auto device = dx12_acquire_native_device(info);
  cgpui::Dx12Ptr<ID3D12InfoQueue> messages;
  return device && SUCCEEDED(device.As(&messages)) && messages &&
      dx12_recovery_debug_messages_safe(messages.Get());
}
cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> pacing_begin_bounded(
    cgpui::Renderer& renderer, Dx12PacingForwardApi& api) {
  using namespace cgpui;
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  const auto timeout = [] {
    return std::unexpected(Error{.code = ErrorCode::frame_acquisition_failed,
        .message = "native first admission exceeded the test's finite three-second deadline"});
  };
  const auto generation = renderer.backend_info().device_generation;
  const auto* snapshot = renderer.last_frame_diagnostic_snapshot();
  const auto* capture = renderer.last_frame_pixels();
  do {
    const auto polls = api.polls;
    const auto pending = api.pending;
    auto frame = renderer.begin_frame(); // The real virtual call; no renderer wrapper.
    if (frame || frame.error().code != ErrorCode::frame_not_ready) return frame;
    if (api.polls != polls + 1 || api.pending != pending + 1 ||
        renderer.backend_info().device_generation != generation ||
        renderer.last_frame_diagnostic_snapshot() != snapshot || renderer.last_frame_pixels() != capture)
      return std::unexpected(Error{.code = ErrorCode::frame_acquisition_failed,
          .message = "native pending begin lacked a real timeout or changed successful state"});
    if (std::chrono::steady_clock::now() >= deadline) return timeout();
    // Test-only bounded dispatch/yield, outside the returned begin operation lock.
    MSG message{};
    for (unsigned count = 0; count != 32 && PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE); ++count) {
      if (message.message == WM_QUIT) {
        PostQuitMessage(static_cast<int>(message.wParam));
        break;
      }
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
    if (std::chrono::steady_clock::now() >= deadline) return timeout();
    Sleep(1);
    if (std::chrono::steady_clock::now() >= deadline) return timeout();
  } while (true);
}
} // namespace cgpui_test
