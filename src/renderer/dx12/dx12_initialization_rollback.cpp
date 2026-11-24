#include "dx12_initialization_rollback_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_composition_internal.hpp"
#include "dx12_native_retirement_internal.hpp"
#include "dx12_recovery_work_internal.hpp"
#include "dx12_window_alpha_internal.hpp"
#include "../../platform/win32_surface_alpha_protocol_internal.hpp"

namespace cgpui {
namespace {
bool unchanged(const Dx12InitializationWindow& window) {
  DWORD process = 0;
  if (!window.hwnd || !window.instance || window.owner != GetCurrentThreadId() ||
      !IsWindow(window.hwnd) || GetWindowThreadProcessId(window.hwnd, &process) != window.owner ||
      process != GetCurrentProcessId() || !GetPropW(window.hwnd, win32_surface_alpha_property)) return false;
  if (static_cast<UINT_PTR>(SendMessageW(window.hwnd,
          win32_surface_alpha_instance_message, 0, 0)) != window.instance) return false;
  return ((GetWindowLongPtrW(window.hwnd, GWL_EXSTYLE) & WS_EX_NOREDIRECTIONBITMAP) != 0) == window.composition;
}
Error refused(const char* reason) {
  return {ErrorCode::renderer_initialization_failed, std::string("DX12 fallback rollback refused: ") + reason};
}
} // namespace
Dx12InitializationWindow dx12_initialization_window(const RenderSurfaceDescriptor& descriptor) {
  const auto* surface = std::get_if<Win32SurfaceHandle>(&descriptor.native_surface);
  if (!surface || !surface->hwnd) return {};
  Dx12InitializationWindow window;
  window.hwnd = static_cast<HWND>(surface->hwnd);
  DWORD process = 0;
  window.owner = GetWindowThreadProcessId(window.hwnd, &process);
  if (!window.owner || process != GetCurrentProcessId() || window.owner != GetCurrentThreadId() ||
      !GetPropW(window.hwnd, win32_surface_alpha_property)) return {};
  window.instance = static_cast<UINT_PTR>(SendMessageW(window.hwnd, win32_surface_alpha_instance_message, 0, 0));
  window.composition = surface->composition;
  return window;
}
Result<void> dx12_rollback_initialization(Dx12State& state, const Dx12InitializationWindow& window) {
  // This is not runtime teardown or a general compositor-completion mechanism.
  if (!unchanged(window) || state.hwnd != window.hwnd || state.operation_depth != 1 ||
      state.submitted || state.frame_active || state.pending_fence_value || state.recovery_pending)
    return std::unexpected(refused("window identity, owner or never-submitted proof is missing"));
  if (state.device && FAILED(state.device->GetDeviceRemovedReason()))
    return std::unexpected(refused("native device loss is not an initialization fallback"));
  if (state.composition_host && (state.composition_host->committed ||
      state.composition_host->retired_chain || state.composition_host->failed_binding))
    return std::unexpected(refused("composition may already be externally visible"));
  if (state.window_alpha && (state.window_alpha->instance != window.instance ||
      state.window_alpha->hwnd != window.hwnd || state.window_alpha->thread != window.owner))
    return std::unexpected(refused("alpha lease identity differs"));
  // A fresh factory may own a DXGI window association after chain creation.
  if (state.factory && state.swapchain && !dx12_composition_route(state.surface_policy)) {
    const HRESULT detached = state.factory->MakeWindowAssociation(nullptr, 0);
    if (FAILED(detached)) return std::unexpected(dx12_error(detached, "initialization window disassociation"));
  }
  // The creation path has never committed this host. Drop its pending graph
  // directly: the runtime cleanup's best-effort Commit is not a rollback proof.
  state.composition_host.reset();
  dx12_release_native_objects(state);
  dx12_cancel_recovery_work(state);
  if (!unchanged(window)) return std::unexpected(refused("window changed during native retirement"));
  if (state.window_alpha) {
    auto& lease = *state.window_alpha;
    auto released = dx12_alpha_protocol(lease.hwnd, win32_surface_alpha_release_message, lease.instance, lease.cookie);
    if (!released) return std::unexpected(released.error());
    lease.cookie = 0; // Explicit release succeeded; suppress destructor's best-effort second send.
    state.window_alpha.reset();
  }
  if (!unchanged(window)) return std::unexpected(refused("window changed during alpha release"));
  // Conservative opaque-history stays sticky; it is not an active native owner
  // and must not be erased merely to conceal that HWND-flip was attempted.
  return {};
}
} // namespace cgpui
