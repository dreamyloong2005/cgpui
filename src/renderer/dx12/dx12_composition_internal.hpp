#pragma once

#include "dx12_native_internal.hpp"
#include "dx12_initialization_failure_internal.hpp"
#include <dcomp.h>
#include <memory>

namespace cgpui {
struct Dx12State;
struct Dx12SwapchainCandidate;
class Dx12CompositionApi;

// Host-only DirectComposition objects. The DX12 state owns its queue and
// swapchain; no D3D11 device, shared texture or cross-API copy is involved.
struct Dx12CompositionHost {
  Dx12CompositionHost();
  ~Dx12CompositionHost();
  Dx12Ptr<IDCompositionDevice> device;
  Dx12Ptr<IDCompositionTarget> target;
  Dx12Ptr<IDCompositionVisual> visual;
  bool committed = false;
  Dx12CompositionApi* api = nullptr;
  std::unique_ptr<Dx12SwapchainCandidate> retired_chain;
  std::unique_ptr<Dx12SwapchainCandidate> failed_binding; // Only on terminal ambiguous binding rollback.
};

Result<void> dx12_create_composition_host(Dx12State& state, Dx12InitializationFailureClass* failure = nullptr);
Result<void> dx12_commit_composition(Dx12State& state);
// After GPU completion/removal, or a partial initialization without submission.
// Detaches content best-effort and always drops every local composition owner.
void dx12_release_composition(Dx12State& state) noexcept;
} // namespace cgpui
