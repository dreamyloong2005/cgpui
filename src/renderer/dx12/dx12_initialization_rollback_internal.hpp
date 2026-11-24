#pragma once
#include "dx12_native_internal.hpp"

namespace cgpui {
struct Dx12State;
struct Dx12InitializationWindow {
  HWND hwnd = nullptr;
  UINT_PTR instance = 0;
  DWORD owner = 0;
  bool composition = false;
};
Dx12InitializationWindow dx12_initialization_window(const RenderSurfaceDescriptor&);
Result<void> dx12_rollback_initialization(Dx12State&, const Dx12InitializationWindow&);
} // namespace cgpui
