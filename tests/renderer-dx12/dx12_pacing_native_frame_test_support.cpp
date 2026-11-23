#include "dx12_pacing_native_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_frame_recovery_internal.hpp"
#include <chrono>
#include <tuple>

namespace cgpui_test {
bool Dx12PacingNativeFixture::await_admission() {
  using namespace cgpui;
  const auto safety = [&] { return std::make_tuple(state->submitted, state->fence_value,
      state->pending_fence_value, state->failed, state->recovery_pending, state->info.device_generation); };
  const auto before = safety();
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  do {
    const auto admitted = pacing.try_admit();
    if (!admitted || before != safety()) return false;
    if (*admitted == Dx12FrameAdmission::ready) return true;
    if (*admitted != Dx12FrameAdmission::pending) return false;
    // Test-only finite polling. The core does not sleep, pump, or spin.
    MSG message{};
    for (unsigned dispatched = 0; dispatched != 64 &&
        PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE); ++dispatched) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
    Sleep(1);
  } while (std::chrono::steady_clock::now() < deadline);
  return false;
}
std::unique_ptr<cgpui::Dx12Frame> Dx12PacingNativeFixture::begin_admitted() {
  if (!cgpui::dx12_prepare_frame(*state) || !await_admission()) return {};
  auto frame = std::make_unique<cgpui::Dx12Frame>(state);
  state->frame_active = true;
  return frame;
}
bool Dx12PacingNativeFixture::present_pixels(cgpui::Dx12Frame& frame, bool green) {
  frame.clear({0, 0, 1, 1});
  frame.draw_rect({.rect = {{8, 8}, {24, 24}},
      .color = green ? cgpui::Color{0, 1, 0, 1} : cgpui::Color{1, 0, 0, 1}});
  if (!frame.request_pixel_capture()) return false;
  const auto presented = frame.present();
  if (!presented) return false;
  // Stage A manual integration: successful native present proves an attempt.
  // Stage B must consume at the real Present call even when that call fails.
  pacing.presentation_attempted();
  return dx12_pixel(state->pixels ? &*state->pixels : nullptr, 16, 16,
      green ? std::array<unsigned char, 4>{0, 255, 0, 255}
            : std::array<unsigned char, 4>{255, 0, 0, 255}) &&
      dx12_pixel(&*state->pixels, 48, 48, {0, 0, 255, 255}) && debug_clean();
}
bool Dx12PacingNativeFixture::debug_clean() const {
  // Mandatory actual InfoQueue (run with the existing DX12 debug control).
  cgpui::Dx12Ptr<ID3D12InfoQueue> messages;
  return SUCCEEDED(state->device.As(&messages)) && messages &&
      dx12_recovery_debug_messages_safe(messages.Get());
}
} // namespace cgpui_test
