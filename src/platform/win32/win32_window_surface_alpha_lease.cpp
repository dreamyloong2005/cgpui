#include "win32_window_surface_alpha_internal.hpp"
#include "win32_native_menu_alpha_internal.hpp"
#include <dwmapi.h>
#include <uxtheme.h>

namespace cgpui {
HRESULT win32_set_redirection_alpha(HWND hwnd, bool enabled) {
  // Set-only attribute; OS gating and the real HRESULT, never a fabricated getter.
  constexpr DWORD redirection_bitmap_alpha = 39;
  const BOOL value = enabled ? TRUE : FALSE;
  return DwmSetWindowAttribute(hwnd, redirection_bitmap_alpha, &value, sizeof(value));
}

HRESULT Win32WindowSurfaceAlpha::reserve(UINT_PTR owner_cookie) {
  if (owner_cookie_) return owner_cookie_ == owner_cookie && reserved_ ? S_FALSE : HRESULT_FROM_WIN32(ERROR_BUSY);
  if (!supported()) return E_NOTIMPL;
  const HRESULT disabled = win32_set_redirection_alpha(hwnd_, false);
  if (FAILED(disabled)) return disabled;
  reserved_ = true;
  owner_cookie_ = owner_cookie;
  repair_result_ = S_OK;
  // No alpha-zero client erase, BufferedPaint initialization or menu repair.
  return S_OK;
}

HRESULT Win32WindowSurfaceAlpha::acquire(UINT_PTR owner_cookie) {
  if (owner_cookie_ && owner_cookie_ != owner_cookie) return HRESULT_FROM_WIN32(ERROR_BUSY);
  if (active_) return S_FALSE;
  // A reservation is not cached alpha capability: opaque chrome/menu changes
  // remain legal, so promotion must recheck the current topology and HMENU gate.
  if (!supported()) return E_NOTIMPL;
  const HRESULT initialized = BufferedPaintInit();
  if (FAILED(initialized)) return initialized;
  const HRESULT enabled = win32_set_redirection_alpha(hwnd_, true);
  if (FAILED(enabled)) { BufferedPaintUnInit(); return enabled; }
  active_ = true;
  owner_cookie_ = owner_cookie;
  repair_result_ = S_OK;
  RECT client{};
  HDC dc = GetDC(hwnd_);
  if (!dc || !GetClientRect(hwnd_, &client) ||
      !FillRect(dc, &client, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)))) repair_result_ = E_FAIL;
  if (dc) ReleaseDC(hwnd_, dc);
  if (SUCCEEDED(repair_result_)) repair_result_ = win32_repair_native_menu_alpha(hwnd_);
  if (FAILED(repair_result_)) {
    const HRESULT failure = repair_result_;
    const HRESULT rollback = reserved_ ? suspend() : release_owner();
    return FAILED(rollback) ? rollback : failure;
  }
  RedrawWindow(hwnd_, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME);
  return S_OK;
}

HRESULT Win32WindowSurfaceAlpha::suspend() {
  if (!active_) return S_FALSE;
  const HRESULT disabled = win32_set_redirection_alpha(hwnd_, false);
  if (FAILED(disabled)) { repair_result_ = disabled; return disabled; }
  active_ = false;
  repair_result_ = S_OK;
  BufferedPaintUnInit();
  RedrawWindow(hwnd_, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME);
  return S_OK;
}

HRESULT Win32WindowSurfaceAlpha::release_owner() {
  const HRESULT disabled = suspend();
  if (FAILED(disabled)) return disabled;
  owner_cookie_ = 0;
  reserved_ = false;
  return S_OK;
}
} // namespace cgpui
