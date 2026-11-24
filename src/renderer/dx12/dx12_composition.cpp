#include "dx12_composition_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_swapchain_candidate_internal.hpp"
#include "dx12_composition_api_internal.hpp"

namespace cgpui {
Dx12CompositionHost::Dx12CompositionHost() = default;
Dx12CompositionHost::~Dx12CompositionHost() = default;
Result<void> dx12_create_composition_host(Dx12State& state, Dx12InitializationFailureClass* failure) {
  if (failure) *failure = Dx12InitializationFailureClass::terminal;
  auto host = std::make_unique<Dx12CompositionHost>();
  // No DComp-created surface is needed: the visual consumes our DXGI swapchain.
  HRESULT result = DCompositionCreateDevice(nullptr, IID_PPV_ARGS(&host->device));
  if (FAILED(result)) return std::unexpected(dx12_initialization_error(result, "composition device", failure));
  result = host->device->CreateTargetForHwnd(state.hwnd, TRUE, &host->target);
  if (FAILED(result)) return std::unexpected(dx12_initialization_error(result, "composition HWND target", failure));
  result = host->device->CreateVisual(&host->visual);
  if (FAILED(result)) return std::unexpected(dx12_initialization_error(result, "composition visual", failure));
  result = host->visual->SetContent(state.swapchain.Get());
  if (FAILED(result)) return std::unexpected(dx12_initialization_error(result, "composition swapchain binding", failure));
  result = host->target->SetRoot(host->visual.Get());
  if (FAILED(result)) return std::unexpected(dx12_initialization_error(result, "composition visual root", failure));
  // The first successfully presented frame commits the visual, not creation.
  state.composition_host = std::move(host);
  return {};
}

Result<void> dx12_commit_composition(Dx12State& state) {
  if (!dx12_composition_route(state.surface_policy)) return {};
  if (!state.composition_host) return std::unexpected(dx12_error(E_UNEXPECTED, "composition host missing"));
  auto& host = *state.composition_host;
  if (host.committed) return {};
  const HRESULT result = dx12_apply_composition(host);
  if (FAILED(result)) return std::unexpected(dx12_error(result, "composition commit"));
  host.committed = true;
  // Called only after successful Present, queue completion and debug checks,
  // before publishing diagnostics/capture. Old visual owners outlive that gate.
  host.retired_chain.reset();
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
  host->failed_binding.reset();
  host->retired_chain.reset();
}
} // namespace cgpui
