#include "win32_window_surface_alpha_internal.hpp"

namespace cgpui {

HRESULT Win32WindowSurfaceAlpha::request(UINT message, UINT_PTR owner_cookie, UINT_PTR instance) {
  if (instance != instance_ || !instance) return E_INVALIDARG;
  if (operating_) return HRESULT_FROM_WIN32(ERROR_BUSY);
  if (message == win32_surface_alpha_query_message) {
    if (owner_cookie) return E_INVALIDARG;
    return supported() ? S_OK : E_NOTIMPL;
  }
  if (!owner_cookie) return E_INVALIDARG;
  operating_ = true;
  HRESULT result = E_INVALIDARG;
  switch (message) {
    case win32_surface_alpha_acquire_message:
      result = acquire(owner_cookie);
      break;
    case win32_surface_alpha_mark_opaque_message:
      result = active_ ? HRESULT_FROM_WIN32(ERROR_BUSY) : S_OK;
      if (SUCCEEDED(result)) opaque_history_ = true;
      break;
    case win32_surface_alpha_release_message:
    case win32_surface_alpha_status_message:
      if (active_ && owner_cookie_ == owner_cookie) {
        result = message == win32_surface_alpha_release_message ? release_owner() : repair_result_;
        if (message == win32_surface_alpha_status_message && !accepts_native_menu() && GetMenu(hwnd_))
          result = E_NOTIMPL;
      }
      break;
    default:
      break;
  }
  operating_ = false;
  return result;
}

} // namespace cgpui
