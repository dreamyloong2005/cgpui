#include "dx12_window_alpha_internal.hpp"
#include "dx12_state_internal.hpp"
#include "../../platform/win32_surface_alpha_protocol_internal.hpp"
#include <atomic>

namespace cgpui {
UINT_PTR dx12_alpha_cookie() {
  static std::atomic<UINT_PTR> sequence{0};
  const auto value = ++sequence;
  return value ? value : ++sequence;
}
static Result<void> validate_window_protocol(HWND hwnd) {
  DWORD process = 0;
  const DWORD thread = GetWindowThreadProcessId(hwnd, &process);
  if (!thread || process != GetCurrentProcessId() || thread != GetCurrentThreadId())
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 ordinary alpha operations require the live HWND's owning thread"});
  if (!GetPropW(hwnd, win32_surface_alpha_property))
    return std::unexpected(dx12_unsupported("ordinary alpha on an unmanaged HWND"));
  return {};
}
Result<HRESULT> dx12_alpha_protocol(HWND hwnd, UINT message, UINT_PTR instance, UINT_PTR cookie) {
  if (auto valid = validate_window_protocol(hwnd); !valid) return std::unexpected(valid.error());
  const HRESULT result = static_cast<HRESULT>(SendMessageW(hwnd, message, cookie, LPARAM(instance)));
  if (result == E_NOTIMPL || result == E_NOINTERFACE)
    return std::unexpected(dx12_unsupported("ordinary alpha OS, window chrome or prior HWND-flip history"));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "ordinary alpha window lease"));
  return result;
}

Result<UINT_PTR> dx12_alpha_window_instance(HWND hwnd) {
  // Validate owner-thread/property before sending even the identity query.
  if (auto valid = validate_window_protocol(hwnd); !valid) return std::unexpected(valid.error());
  const auto instance = static_cast<UINT_PTR>(SendMessageW(hwnd, win32_surface_alpha_instance_message, 0, 0));
  if (!instance) return std::unexpected(dx12_unsupported("ordinary alpha window identity"));
  if (auto valid = dx12_alpha_protocol(hwnd, win32_surface_alpha_query_message, instance); !valid)
    return std::unexpected(valid.error());
  return instance;
}

static Result<DWORD_PTR> history_message(HWND hwnd, UINT message, UINT_PTR cookie, UINT_PTR instance) {
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
  (void)dx12_alpha_protocol(hwnd, win32_surface_alpha_release_message, instance, cookie);
}

Result<void> dx12_acquire_window_alpha(Dx12State& state) {
  if (state.window_alpha) {
    auto& lease = *state.window_alpha;
    auto acquired = dx12_alpha_protocol(lease.hwnd, win32_surface_alpha_acquire_message, lease.instance, lease.cookie);
    if (!acquired) {
      if (!dx12_validate_window_alpha(state)) state.failed = true; // Failed native rollback is not a usable reservation.
      return std::unexpected(acquired.error());
    }
    if (auto valid = dx12_validate_window_alpha(state); !valid) { state.failed = true; return valid; }
    lease.enabled = true;
    return {};
  }
  auto instance = dx12_alpha_window_instance(state.hwnd);
  if (!instance) return std::unexpected(instance.error());
  auto lease = std::make_unique<Dx12WindowAlphaLease>();
  lease->hwnd = state.hwnd;
  lease->instance = *instance;
  lease->thread = GetCurrentThreadId();
  lease->cookie = dx12_alpha_cookie();
  if (auto result = dx12_alpha_protocol(state.hwnd, win32_surface_alpha_acquire_message, lease->instance, lease->cookie); !result)
    return std::unexpected(result.error());
  if (auto result = dx12_alpha_protocol(state.hwnd, win32_surface_alpha_status_message, lease->instance, lease->cookie); !result)
    return std::unexpected(result.error());
  lease->enabled = true;
  state.window_alpha = std::move(lease);
  return {};
}

Result<void> dx12_mark_opaque_window(Dx12State& state) {
  if (dx12_composition_route(state.surface_policy) || !GetPropW(state.hwnd, win32_surface_alpha_property)) return {};
  // Unsupported chrome/OS still has an instance; recording a conservative flip
  // attempt prevents a later chrome change from exposing a stale HWND frame.
  auto instance = history_message(state.hwnd, win32_surface_alpha_instance_message, 0, 0);
  if (!instance) return std::unexpected(instance.error());
  if (!*instance) return std::unexpected(dx12_unsupported("ordinary HWND flip-history identity"));
  auto result = history_message(state.hwnd, win32_surface_alpha_mark_opaque_message, dx12_alpha_cookie(), *instance);
  if (!result) return std::unexpected(result.error());
  if (FAILED(static_cast<HRESULT>(*result)))
    return std::unexpected(dx12_error(static_cast<HRESULT>(*result), "ordinary HWND flip-history mark"));
  return {};
}

Result<void> dx12_validate_window_alpha(const Dx12State& state) {
  if (state.surface_changing && state.operation_depth > 1)
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 reentrant frame/surface operations during alpha replacement are not permitted"});
  if (!state.window_alpha) return {};
  const auto& lease = *state.window_alpha;
  if (!lease.enabled && lease.thread != GetCurrentThreadId()) {
    // Opaque reservation is not an alpha-thread restriction. A worker may use
    // the original opaque contract while the UI pumps this bounded token query.
    auto status = history_message(lease.hwnd, win32_surface_alpha_status_message, lease.cookie, lease.instance);
    if (!status) return std::unexpected(status.error());
    if (FAILED(static_cast<HRESULT>(*status)))
      return std::unexpected(dx12_error(static_cast<HRESULT>(*status), "opaque composition reservation status"));
    return {};
  }
  if (auto result = dx12_alpha_protocol(lease.hwnd, win32_surface_alpha_status_message, lease.instance, lease.cookie); !result)
    return std::unexpected(result.error());
  return {};
}
} // namespace cgpui
