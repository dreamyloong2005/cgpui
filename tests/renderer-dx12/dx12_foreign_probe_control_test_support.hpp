#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>

namespace cgpui_test {
enum class ForeignProbeMode { pumping, parked };
enum class ForeignProbeWait { signaled, cancelled, timeout, failed };
inline constexpr UINT foreign_probe_pulse = WM_APP + 91U;
class ForeignProbeHandle {
 public:
  explicit ForeignProbeHandle(HANDLE value = nullptr) noexcept;
  ~ForeignProbeHandle();
  ForeignProbeHandle(const ForeignProbeHandle&) = delete;
  ForeignProbeHandle& operator=(const ForeignProbeHandle&) = delete;
  HANDLE get() const noexcept;
  HANDLE* put() noexcept;
 private:
  HANDLE value_;
};
const char* foreign_probe_mode_name(ForeignProbeMode mode) noexcept;
ForeignProbeWait foreign_probe_wait(HANDLE event, HANDLE cancel, bool pump,
    DWORD milliseconds, HANDLE pulse_seen = nullptr);
void foreign_probe_log(const char* mode, const char* stage, DWORD owner = 0,
    std::uint64_t luid = 0, std::uint64_t generation = 0, int result = 0) noexcept;
bool foreign_probe_output_ok() noexcept;
int foreign_probe_process(ForeignProbeMode mode);
int foreign_probe_case(ForeignProbeMode mode, HANDLE cancel);
} // namespace cgpui_test
