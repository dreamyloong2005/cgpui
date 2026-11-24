#include "dx12_window_alpha_internal.hpp"
#include "dx12_state_internal.hpp"
#include "../../platform/win32_surface_alpha_protocol_internal.hpp"

namespace cgpui {
Result<bool> dx12_reserve_ordinary_composition(Dx12State& state) {
  DWORD process = 0;
  // Existing raw/foreign-thread opaque factories must keep their old flip path.
  if (GetWindowThreadProcessId(state.hwnd, &process) != GetCurrentThreadId() ||
      process != GetCurrentProcessId() || !GetPropW(state.hwnd, win32_surface_alpha_property)) return false;
  auto instance = dx12_alpha_window_instance(state.hwnd);
  if (!instance) {
    if (instance.error().code == ErrorCode::unsupported_platform) return false;
    return std::unexpected(instance.error());
  }
  auto lease = std::make_unique<Dx12WindowAlphaLease>();
  lease->hwnd = state.hwnd; lease->instance = *instance;
  lease->thread = GetCurrentThreadId(); lease->cookie = dx12_alpha_cookie();
  auto reserved = dx12_alpha_protocol(state.hwnd, win32_surface_alpha_reserve_message, *instance, lease->cookie);
  if (!reserved) {
    if (reserved.error().code == ErrorCode::unsupported_platform) return false;
    return std::unexpected(reserved.error());
  }
  state.window_alpha = std::move(lease);
  return true;
}

Result<void> dx12_suspend_window_alpha(Dx12State& state) {
  if (!state.window_alpha) return std::unexpected(dx12_error(E_UNEXPECTED, "alpha reservation missing"));
  auto& lease = *state.window_alpha;
  if (auto result = dx12_alpha_protocol(lease.hwnd, win32_surface_alpha_suspend_message, lease.instance, lease.cookie); !result) {
    state.failed = true;
    return std::unexpected(result.error());
  }
  lease.enabled = false;
  return {};
}
} // namespace cgpui
