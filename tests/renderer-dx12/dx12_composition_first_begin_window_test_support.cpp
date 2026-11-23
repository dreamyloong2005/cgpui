#include "dx12_composition_first_begin_test_support.hpp"

namespace cgpui_test {
Dx12CompositionFirstBeginWindows::Dx12CompositionFirstBeginWindows()
    : active_before_(GetActiveWindow()) {
  RECT work{};
  if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0) ||
      work.right - work.left < 192 || work.bottom - work.top < 192) return;
  const HINSTANCE instance = GetModuleHandleW(nullptr);
  // Both real windows start hidden; no pump, implicit show, or renderer wrapper.
  owner_ = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, L"STATIC",
      L"CGPUI composition first-begin owner", WS_POPUP,
      work.left + 16, work.top + 16, 96, 96, nullptr, nullptr, instance, nullptr);
  if (!owner_) return;
  popup_ = CreateWindowExW(WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_NOREDIRECTIONBITMAP,
      L"STATIC", L"CGPUI composition first-begin popup", WS_POPUP,
      work.left + 32, work.top + 32, 64, 64, owner_, nullptr, instance, nullptr);
}

Dx12CompositionFirstBeginWindows::~Dx12CompositionFirstBeginWindows() {
  if (popup_) DestroyWindow(popup_);
  if (owner_) DestroyWindow(owner_);
}

bool Dx12CompositionFirstBeginWindows::valid() const {
  return owner_ && popup_ && IsWindow(owner_) && IsWindow(popup_);
}

cgpui::RenderSurfaceDescriptor Dx12CompositionFirstBeginWindows::descriptor() const {
  return {.native_surface = cgpui::Win32SurfaceHandle{
              .hinstance = GetModuleHandleW(nullptr), .hwnd = popup_, .composition = true},
      .framebuffer_size = {64, 64}, .scale = {1}, .transparent_background = true};
}
} // namespace cgpui_test
