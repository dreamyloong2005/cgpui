#include "dx12_foreign_probe_control_test_support.hpp"
#include <atomic>
#include <cstdio>

namespace cgpui_test {
namespace { std::atomic<bool> output_ok{true}; }
ForeignProbeHandle::ForeignProbeHandle(HANDLE value) noexcept : value_(value) {}
ForeignProbeHandle::~ForeignProbeHandle() {
  if (value_ && value_ != INVALID_HANDLE_VALUE) CloseHandle(value_);
}
HANDLE ForeignProbeHandle::get() const noexcept { return value_; }
HANDLE* ForeignProbeHandle::put() noexcept { return &value_; }
const char* foreign_probe_mode_name(ForeignProbeMode mode) noexcept {
  return mode == ForeignProbeMode::pumping ? "pumping" : "parked";
}
void foreign_probe_log(const char* mode, const char* stage, DWORD owner,
    std::uint64_t luid, std::uint64_t generation, int result) noexcept {
  char buffer[512]{};
  const int length = std::snprintf(buffer, sizeof(buffer),
      "DX12_FOREIGN_PROBE mode=%s stage=%s caller=%lu owner=%lu luid=%llu generation=%llu tick=%llu result=%d\n",
      mode, stage, GetCurrentThreadId(), owner, static_cast<unsigned long long>(luid),
      static_cast<unsigned long long>(generation), static_cast<unsigned long long>(GetTickCount64()), result);
  DWORD written = 0;
  if (length <= 0 || length >= static_cast<int>(sizeof(buffer)) ||
      !WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buffer, static_cast<DWORD>(length), &written, nullptr) ||
      written != static_cast<DWORD>(length)) output_ok.store(false);
}
bool foreign_probe_output_ok() noexcept { return output_ok.load(); }
ForeignProbeWait foreign_probe_wait(HANDLE event, HANDLE cancel, bool pump,
    DWORD milliseconds, HANDLE pulse_seen) {
  HANDLE handles[2]{};
  DWORD count = 0, cancel_index = MAXDWORD, event_index = MAXDWORD;
  if (cancel) { cancel_index = count; handles[count++] = cancel; }
  if (event) { event_index = count; handles[count++] = event; }
  const auto started = GetTickCount64();
  for (;;) {
    const auto elapsed = GetTickCount64() - started;
    const DWORD remaining = milliseconds == INFINITE ? INFINITE :
        elapsed >= milliseconds ? 0 : milliseconds - static_cast<DWORD>(elapsed);
    // Parked mode uses a non-alertable kernel wait, never message retrieval.
    const DWORD result = pump
        ? MsgWaitForMultipleObjectsEx(count, handles, remaining, QS_ALLINPUT, MWMO_INPUTAVAILABLE)
        : WaitForMultipleObjects(count, handles, FALSE, remaining);
    if (cancel && result == WAIT_OBJECT_0 + cancel_index) return ForeignProbeWait::cancelled;
    if (event && result == WAIT_OBJECT_0 + event_index) return ForeignProbeWait::signaled;
    if (result == WAIT_TIMEOUT) return ForeignProbeWait::timeout;
    if (!pump || result != WAIT_OBJECT_0 + count) return ForeignProbeWait::failed;
    MSG message{};
    for (unsigned n = 0; n != 32 && PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE); ++n) {
      if (message.message == WM_QUIT) return ForeignProbeWait::failed;
      if (!message.hwnd && message.message == foreign_probe_pulse) {
        if (pulse_seen) SetEvent(pulse_seen);
      } else {
        TranslateMessage(&message);
        DispatchMessageW(&message);
      }
    }
    if (milliseconds != INFINITE && GetTickCount64() - started >= milliseconds)
      return ForeignProbeWait::timeout;
  }
}
} // namespace cgpui_test
