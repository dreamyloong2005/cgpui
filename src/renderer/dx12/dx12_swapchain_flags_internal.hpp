#pragma once

#include "dx12_native_internal.hpp"

namespace cgpui {
// Both presentation routes require admission. Same-chain resize must retain
// the creation flags; unsupported native initialization remains a real error.
constexpr UINT dx12_swapchain_flags() noexcept {
  return DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
}
} // namespace cgpui
