#pragma once

#include "dx12_native_internal.hpp"
#include "dx12_frame_pacing_internal.hpp"
#include <array>
#include <memory>

namespace cgpui {
struct Dx12State;
// One fully prepared, never-submitted replacement, or one safely retired former
// chain retained until the new visual's successful frame Commit/detachment.
struct Dx12SwapchainCandidate {
  Dx12Ptr<IDXGISwapChain3> swapchain;
  Dx12Ptr<ID3D12DescriptorHeap> rtv_heap;
  std::array<Dx12Ptr<ID3D12Resource>, 2> targets;
  UINT rtv_stride = 0;
  std::unique_ptr<Dx12FramePacing> pacing; // Closes its handle before releasing the chain.
};
Result<std::unique_ptr<Dx12SwapchainCandidate>> dx12_prepare_composition_chain(Dx12State&, bool premultiplied);
void dx12_exchange_swapchain(Dx12State&, Dx12SwapchainCandidate&) noexcept;
} // namespace cgpui
