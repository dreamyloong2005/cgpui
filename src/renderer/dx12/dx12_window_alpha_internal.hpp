#pragma once

#include "dx12_native_internal.hpp"

namespace cgpui {
struct Dx12State;
// Window-level ownership survives native GPU epochs and outstanding frames.
// Destruction releases it only after composition and swapchain retirement.
struct Dx12WindowAlphaLease {
  HWND hwnd = nullptr;
  UINT_PTR instance = 0;
  UINT_PTR cookie = 0;
  DWORD thread = 0;
  ~Dx12WindowAlphaLease();
};
Result<void> dx12_acquire_window_alpha(Dx12State& state);
Result<void> dx12_mark_opaque_window(Dx12State& state);
Result<void> dx12_validate_window_alpha(const Dx12State& state);
} // namespace cgpui
