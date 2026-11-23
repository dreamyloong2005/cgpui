#include "dx12_composition_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
Result<void> dx12_create_composition_host(Dx12State& state) {
  auto host = std::make_unique<Dx12CompositionHost>();
  // No DComp-created surface is needed: the visual consumes our DXGI swapchain.
  HRESULT result = DCompositionCreateDevice(nullptr, IID_PPV_ARGS(&host->device));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "composition device"));
  result = host->device->CreateTargetForHwnd(state.hwnd, TRUE, &host->target);
  if (FAILED(result)) return std::unexpected(dx12_error(result, "composition HWND target"));
  result = host->device->CreateVisual(&host->visual);
  if (FAILED(result)) return std::unexpected(dx12_error(result, "composition visual"));
  result = host->visual->SetContent(state.swapchain.Get());
  if (FAILED(result)) return std::unexpected(dx12_error(result, "composition swapchain binding"));
  result = host->target->SetRoot(host->visual.Get());
  if (FAILED(result)) return std::unexpected(dx12_error(result, "composition visual root"));
  // The first successfully presented frame commits the visual, not creation.
  state.composition_host = std::move(host);
  return {};
}

Result<void> dx12_commit_composition(Dx12State& state) {
  if (!state.composition_surface) return {};
  if (!state.composition_host) return std::unexpected(dx12_error(E_UNEXPECTED, "composition host missing"));
  auto& host = *state.composition_host;
  if (host.committed) return {};
  const HRESULT result = host.device->Commit();
  if (FAILED(result)) return std::unexpected(dx12_error(result, "composition commit"));
  host.committed = true;
  return {};
}

void dx12_release_composition(Dx12State& state) noexcept {
  if (!state.composition_host) return;
  auto host = std::move(state.composition_host);
  // Removed devices may reject detachment/commit. Local references must still
  // be released; do not turn compositor completion into an unbounded GPU wait.
  if (host->visual) (void)host->visual->SetContent(nullptr);
  if (host->target) (void)host->target->SetRoot(nullptr);
  if (host->device) (void)host->device->Commit();
  host->visual.Reset();
  host->target.Reset();
  host->device.Reset();
}
} // namespace cgpui
