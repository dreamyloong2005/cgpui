#pragma once

#include "dx12_native_internal.hpp"
#include <dcomp.h>

namespace cgpui {
struct Dx12State;

// Host-only DirectComposition objects. The DX12 state owns its queue and
// swapchain; no D3D11 device, shared texture or cross-API copy is involved.
struct Dx12CompositionHost {
  Dx12Ptr<IDCompositionDevice> device;
  Dx12Ptr<IDCompositionTarget> target;
  Dx12Ptr<IDCompositionVisual> visual;
  bool committed = false;
};

Result<void> dx12_create_composition_host(Dx12State& state);
Result<void> dx12_commit_composition(Dx12State& state);
// After GPU completion/removal, or a partial initialization without submission.
// Detaches content best-effort and always drops every local composition owner.
void dx12_release_composition(Dx12State& state) noexcept;
} // namespace cgpui
