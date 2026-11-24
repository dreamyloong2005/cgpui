#include "dx12_foreign_probe_native_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"
#include "../../src/renderer/dx12/dx12_frame_admission_internal.hpp"
#include "../../src/renderer/dx12/dx12_frame_recovery_internal.hpp"

namespace cgpui_test {
bool ForeignProbeFixture::capture(bool origin, bool recovered, HANDLE cancel) {
  using namespace cgpui;
  std::unique_ptr<Dx12Frame> frame;
  const auto started = GetTickCount64();
  while (!frame) {
    if (WaitForSingleObject(cancel, 0) != WAIT_TIMEOUT || GetTickCount64() - started >= 3000) return false;
    {
      Dx12OperationGuard operation(*state);
      if (!dx12_prepare_frame(*state)) return false;
      const auto admitted = dx12_admit_frame(*state);
      if (admitted) {
        frame = std::make_unique<Dx12Frame>(state, admitted->admission_call_ns);
        state->frame_active = true;
      } else if (admitted.error().code != ErrorCode::frame_not_ready) return false;
    }
    if (!frame && foreign_probe_wait(nullptr, cancel, true, 2) != ForeignProbeWait::timeout) return false;
  }
  const Color clear = origin ? (recovered ? Color{0, 0, 1, 1} : Color{1, 0, 0, 1})
                             : (recovered ? Color{1, 0, 1, 1} : Color{0, 1, 0, 1});
  const Color rect = origin ? (recovered ? Color{0, 1, 1, 1} : Color{1, 1, 0, 1})
                            : (recovered ? Color{1, 0, 0, 1} : Color{0, 0, 1, 1});
  frame->clear(clear);
  frame->draw_rect({.rect = {{16, 16}, {24, 24}}, .color = rect});
  if (!frame->request_pixel_capture() || !frame->present()) return false;
  frame.reset(); // No caller gate is held while dropping this shared-state pin.
  Dx12OperationGuard operation(*state);
  const auto bytes = [](Color value) {
    return std::array<unsigned char, 4>{static_cast<unsigned char>(value.r * 255),
        static_cast<unsigned char>(value.g * 255), static_cast<unsigned char>(value.b * 255), 255};
  };
  const auto* pixels = state->pixels ? &*state->pixels : nullptr;
  return pixels && pixels->width == 64 && pixels->height == 64 &&
      dx12_pixel(pixels, 8, 8, bytes(clear), 0) && dx12_pixel(pixels, 20, 20, bytes(rect), 0) &&
      dx12_pixel(pixels, 56, 56, bytes(clear), 0) && !state->submitted &&
      !state->frame_active && dx12_check_debug_errors(*state).has_value();
}
} // namespace cgpui_test
