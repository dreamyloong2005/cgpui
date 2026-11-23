#include "win32_window_surface_alpha_internal.hpp"

#include "win32_native_menu_alpha_internal.hpp"
#include "win32_window_backdrop_internal.hpp"
#include <dwmapi.h>
#include <uxtheme.h>
#include <atomic>

namespace cgpui {
namespace {

// Numeric spelling permits older SDKs; runtime build and HRESULT both gate use.
constexpr DWORD redirection_bitmap_alpha = 39;
constexpr std::uint32_t minimum_build = 26100;
std::atomic<UINT_PTR> next_instance{1};

bool compatible_chrome(WindowChromeOptions options) {
  return options.decorations && options.titlebar_visible &&
      options.title_bar == WindowTitleBarMode::system;
}

HRESULT set_alpha(HWND hwnd, BOOL enabled) {
  return DwmSetWindowAttribute(hwnd, redirection_bitmap_alpha, &enabled, sizeof(enabled));
}

} // namespace

Win32WindowSurfaceAlpha::~Win32WindowSurfaceAlpha() { detach(); }

void Win32WindowSurfaceAlpha::attach(HWND hwnd) {
  detach();
  hwnd_ = hwnd;
  instance_ = next_instance.fetch_add(1, std::memory_order_relaxed);
  if (instance_ == 0) instance_ = next_instance.fetch_add(1, std::memory_order_relaxed);
  opaque_history_ = false;
  if (!SetPropW(hwnd_, win32_surface_alpha_property, this)) {
    hwnd_ = nullptr;
    instance_ = 0;
  }
}

void Win32WindowSurfaceAlpha::detach() {
  if (hwnd_ != nullptr && GetPropW(hwnd_, win32_surface_alpha_property) == this) {
    if (active_) (void)set_alpha(hwnd_, FALSE);
    RemovePropW(hwnd_, win32_surface_alpha_property);
  }
  if (active_) BufferedPaintUnInit();
  active_ = false;
  owner_cookie_ = 0;
  hwnd_ = nullptr;
  instance_ = 0;
}

void Win32WindowSurfaceAlpha::set_chrome(WindowChromeOptions options) {
  compatible_chrome_ = compatible_chrome(options);
}

bool Win32WindowSurfaceAlpha::accepts_chrome(WindowChromeOptions options) const {
  return !active_ || compatible_chrome(options);
}

bool Win32WindowSurfaceAlpha::active() const { return active_; }
UINT_PTR Win32WindowSurfaceAlpha::instance() const { return instance_; }

bool Win32WindowSurfaceAlpha::supported() const {
  if (!hwnd_ || !compatible_chrome_ || opaque_history_ ||
      win32_os_build_number() < minimum_build) return false;
  const auto style = GetWindowLongPtrW(hwnd_, GWL_STYLE);
  const auto extended = GetWindowLongPtrW(hwnd_, GWL_EXSTYLE);
  return (style & WS_CAPTION) == WS_CAPTION && !(style & (WS_CHILD | WS_POPUP)) &&
      !(extended & (WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP));
}

HRESULT Win32WindowSurfaceAlpha::acquire(UINT_PTR owner_cookie) {
  if (active_) return owner_cookie_ == owner_cookie ? S_FALSE : HRESULT_FROM_WIN32(ERROR_BUSY);
  if (!supported()) return E_NOTIMPL;
  const HRESULT initialized = BufferedPaintInit();
  if (FAILED(initialized)) return initialized;
  const HRESULT enabled = set_alpha(hwnd_, TRUE);
  if (FAILED(enabled)) {
    BufferedPaintUnInit();
    return enabled;
  }
  active_ = true;
  owner_cookie_ = owner_cookie;
  repair_result_ = S_OK;
  // Clear the redirected client without invoking application callbacks. Future
  // erase messages are routed to the same alpha-zero black by the proc leaf.
  RECT client{};
  HDC dc = GetDC(hwnd_);
  if (!dc || !GetClientRect(hwnd_, &client) ||
      !FillRect(dc, &client, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)))) {
    repair_result_ = E_FAIL;
  }
  if (dc) ReleaseDC(hwnd_, dc);
  if (SUCCEEDED(repair_result_)) repair_result_ = win32_repair_native_menu_alpha(hwnd_);
  if (FAILED(repair_result_)) {
    const HRESULT failure = repair_result_;
    (void)release_owner();
    return failure;
  }
  RedrawWindow(hwnd_, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME);
  return S_OK;
}

HRESULT Win32WindowSurfaceAlpha::release_owner() {
  const HRESULT disabled = set_alpha(hwnd_, FALSE);
  if (FAILED(disabled)) return disabled;
  active_ = false;
  owner_cookie_ = 0;
  BufferedPaintUnInit();
  RedrawWindow(hwnd_, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME);
  return S_OK;
}

void Win32WindowSurfaceAlpha::repair_menu() {
  if (!active_ || repairing_) return;
  repairing_ = true;
  const HRESULT repaired = win32_repair_native_menu_alpha(hwnd_);
  if (FAILED(repaired)) repair_result_ = repaired;
  repairing_ = false;
}

Win32WindowSurfaceAlpha* win32_window_surface_alpha(HWND hwnd) {
  return static_cast<Win32WindowSurfaceAlpha*>(GetPropW(hwnd, win32_surface_alpha_property));
}

} // namespace cgpui
