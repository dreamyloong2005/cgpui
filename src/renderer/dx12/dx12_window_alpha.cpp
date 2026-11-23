#include "dx12_window_alpha_internal.hpp"
#include "dx12_state_internal.hpp"
#include "../../platform/win32_surface_alpha_protocol_internal.hpp"
#include <atomic>

namespace cgpui {
namespace {
UINT_PTR next_cookie() {
  static std::atomic<UINT_PTR> sequence{0};
  const auto value = ++sequence;
  return value ? value : ++sequence;
}
Result<void> validate_window_protocol(HWND hwnd) {
  DWORD process = 0;
  const DWORD thread = GetWindowThreadProcessId(hwnd, &process);
  if (!thread || process != GetCurrentProcessId() || thread != GetCurrentThreadId())
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 ordinary alpha operations require the live HWND's owning thread"});
  if (!GetPropW(hwnd, win32_surface_alpha_property))
    return std::unexpected(dx12_unsupported("ordinary alpha on an unmanaged HWND"));
  return {};
}
Result<HRESULT> protocol(HWND hwnd, UINT message, UINT_PTR instance, UINT_PTR cookie = 0) {
  if (auto valid = validate_window_protocol(hwnd); !valid) return std::unexpected(valid.error());
  const HRESULT result = static_cast<HRESULT>(SendMessageW(hwnd, message, cookie, LPARAM(instance)));
  if (result == E_NOTIMPL || result == E_NOINTERFACE)
    return std::unexpected(dx12_unsupported("ordinary alpha OS, window chrome or prior HWND-flip history"));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "ordinary alpha window lease"));
  return result;
}

Result<UINT_PTR> window_instance(HWND hwnd) {
  // Validate owner-thread/property before sending even the identity query.
  if (auto valid = validate_window_protocol(hwnd); !valid) return std::unexpected(valid.error());
  const auto instance = static_cast<UINT_PTR>(SendMessageW(hwnd, win32_surface_alpha_instance_message, 0, 0));
  if (!instance) return std::unexpected(dx12_unsupported("ordinary alpha window identity"));
  if (auto valid = protocol(hwnd, win32_surface_alpha_query_message, instance); !valid)
    return std::unexpected(valid.error());
  return instance;
}

Result<DWORD_PTR> history_message(HWND hwnd, UINT message, UINT_PTR cookie, UINT_PTR instance) {
  DWORD process = 0;
  if (!GetWindowThreadProcessId(hwnd, &process) || process != GetCurrentProcessId() ||
      !GetPropW(hwnd, win32_surface_alpha_property))
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 opaque history requires a live managed HWND in this process"});
  DWORD_PTR result = 0;
  SetLastError(ERROR_SUCCESS);
  // Opaque factories retain cross-thread support when the owner pumps messages.
  // A timeout fails closed before creating a flip chain; a late mark is harmless.
  if (!SendMessageTimeoutW(hwnd, message, cookie, LPARAM(instance),
                          SMTO_ABORTIFHUNG | SMTO_ERRORONEXIT | SMTO_BLOCK, 1000, &result)) {
    const DWORD error = GetLastError();
    return std::unexpected(dx12_error(HRESULT_FROM_WIN32(error ? error : ERROR_TIMEOUT),
                                     "ordinary HWND flip-history handshake"));
  }
  return result;
}
} // namespace

Dx12WindowAlphaLease::~Dx12WindowAlphaLease() {
  if (!cookie || !IsWindow(hwnd) || !GetPropW(hwnd, win32_surface_alpha_property)) return;
  DWORD process = 0;
  if (GetWindowThreadProcessId(hwnd, &process) != thread || process != GetCurrentProcessId()) return;
  if (thread != GetCurrentThreadId()) {
    // No pointer payload or cross-thread synchronous callback during destruction.
    // The UI thread validates both tokens before touching a possibly reused HWND.
    (void)PostMessageW(hwnd, win32_surface_alpha_release_message, cookie, LPARAM(instance));
    return;
  }
  (void)protocol(hwnd, win32_surface_alpha_release_message, instance, cookie);
}

Result<void> dx12_acquire_window_alpha(Dx12State& state) {
  auto instance = window_instance(state.hwnd);
  if (!instance) return std::unexpected(instance.error());
  auto lease = std::make_unique<Dx12WindowAlphaLease>();
  lease->hwnd = state.hwnd;
  lease->instance = *instance;
  lease->thread = GetCurrentThreadId();
  lease->cookie = next_cookie();
  if (auto result = protocol(state.hwnd, win32_surface_alpha_acquire_message, lease->instance, lease->cookie); !result)
    return std::unexpected(result.error());
  if (auto result = protocol(state.hwnd, win32_surface_alpha_status_message, lease->instance, lease->cookie); !result)
    return std::unexpected(result.error());
  state.window_alpha = std::move(lease);
  return {};
}

Result<void> dx12_mark_opaque_window(Dx12State& state) {
  if (state.composition_surface || !GetPropW(state.hwnd, win32_surface_alpha_property)) return {};
  // Unsupported chrome/OS still has an instance; recording a conservative flip
  // attempt prevents a later chrome change from exposing a stale HWND frame.
  auto instance = history_message(state.hwnd, win32_surface_alpha_instance_message, 0, 0);
  if (!instance) return std::unexpected(instance.error());
  if (!*instance) return std::unexpected(dx12_unsupported("ordinary HWND flip-history identity"));
  auto result = history_message(state.hwnd, win32_surface_alpha_mark_opaque_message, next_cookie(), *instance);
  if (!result) return std::unexpected(result.error());
  if (FAILED(static_cast<HRESULT>(*result)))
    return std::unexpected(dx12_error(static_cast<HRESULT>(*result), "ordinary HWND flip-history mark"));
  return {};
}

Result<void> dx12_validate_window_alpha(const Dx12State& state) {
  if (!state.window_alpha) return {};
  const auto& lease = *state.window_alpha;
  if (auto result = protocol(lease.hwnd, win32_surface_alpha_status_message, lease.instance, lease.cookie); !result)
    return std::unexpected(result.error());
  return {};
}
} // namespace cgpui
