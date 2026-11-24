#pragma once

#include "../win32_surface_alpha_protocol_internal.hpp"
#include "cgpui/core/window.hpp"

namespace cgpui {

// Independent of backdrop/material selection. This owner survives renderer
// device recovery, but never survives destruction/reuse of its HWND.
class Win32WindowSurfaceAlpha {
 public:
  Win32WindowSurfaceAlpha() = default;
  ~Win32WindowSurfaceAlpha();
  Win32WindowSurfaceAlpha(const Win32WindowSurfaceAlpha&) = delete;
  Win32WindowSurfaceAlpha& operator=(const Win32WindowSurfaceAlpha&) = delete;

  void attach(HWND hwnd);
  void detach();
  void set_chrome(WindowChromeOptions options);
  [[nodiscard]] bool accepts_chrome(WindowChromeOptions options) const;
  [[nodiscard]] bool accepts_native_menu() const;
  [[nodiscard]] bool active() const;
  [[nodiscard]] UINT_PTR instance() const;
  HRESULT request(UINT message, UINT_PTR owner_cookie, UINT_PTR instance);
  void repair_menu();

 private:
  [[nodiscard]] bool supported() const;
  HRESULT reserve(UINT_PTR owner_cookie);
  HRESULT acquire(UINT_PTR owner_cookie);
  HRESULT suspend();
  HRESULT release_owner();
  HWND hwnd_ = nullptr;
  UINT_PTR instance_ = 0;
  UINT_PTR owner_cookie_ = 0;
  HRESULT repair_result_ = S_OK;
  WindowTitleBarMode title_bar_ = WindowTitleBarMode::system;
  bool compatible_chrome_ = true;
  bool opaque_history_ = false;
  bool reserved_ = false;
  bool active_ = false;
  bool operating_ = false;
  bool repairing_ = false;
};

[[nodiscard]] Win32WindowSurfaceAlpha* win32_window_surface_alpha(HWND hwnd);
HRESULT win32_set_redirection_alpha(HWND hwnd, bool enabled);

} // namespace cgpui
