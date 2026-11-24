#include "win32_window_surface_alpha_internal.hpp"

#include "win32_native_menu_alpha_internal.hpp"
#include "win32_native_menu_policy_internal.hpp"
#include "win32_window_backdrop_internal.hpp"
#include <uxtheme.h>
#include <atomic>

namespace cgpui {
namespace {

constexpr std::uint32_t minimum_build = 26100;
std::atomic<UINT_PTR> next_instance{1};

bool compatible_chrome(WindowChromeOptions options) {
  return options.decorations && options.titlebar_visible &&
      (options.title_bar == WindowTitleBarMode::system || options.title_bar == WindowTitleBarMode::client);
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
    if (active_) (void)win32_set_redirection_alpha(hwnd_, false);
    RemovePropW(hwnd_, win32_surface_alpha_property);
  }
  if (active_) BufferedPaintUnInit();
  active_ = false;
  reserved_ = false;
  owner_cookie_ = 0;
  hwnd_ = nullptr;
  instance_ = 0;
}

void Win32WindowSurfaceAlpha::set_chrome(WindowChromeOptions options) {
  compatible_chrome_ = compatible_chrome(options);
  title_bar_ = options.title_bar;
}

bool Win32WindowSurfaceAlpha::accepts_chrome(WindowChromeOptions options) const {
  return !active_ || (compatible_chrome(options) && options.title_bar == title_bar_);
}

bool Win32WindowSurfaceAlpha::active() const { return active_; }
UINT_PTR Win32WindowSurfaceAlpha::instance() const { return instance_; }
bool Win32WindowSurfaceAlpha::accepts_native_menu() const {
  return !active_ || title_bar_ != WindowTitleBarMode::client;
}

bool Win32WindowSurfaceAlpha::supported() const {
  if (!hwnd_ || !compatible_chrome_ || opaque_history_ ||
      win32_os_build_number() < minimum_build) return false;
  if (title_bar_ == WindowTitleBarMode::client &&
      (GetMenu(hwnd_) || win32_native_menu_installation_active())) return false;
  const auto style = GetWindowLongPtrW(hwnd_, GWL_STYLE);
  const auto extended = GetWindowLongPtrW(hwnd_, GWL_EXSTYLE);
  return (style & WS_CAPTION) == WS_CAPTION && !(style & (WS_CHILD | WS_POPUP)) &&
      !(extended & (WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP));
}

void Win32WindowSurfaceAlpha::repair_menu() {
  if (!active_ || repairing_) return;
  if (!accepts_native_menu()) {
    if (GetMenu(hwnd_)) repair_result_ = E_NOTIMPL;
    return;
  }
  repairing_ = true;
  const HRESULT repaired = win32_repair_native_menu_alpha(hwnd_);
  if (FAILED(repaired)) repair_result_ = repaired;
  repairing_ = false;
}

Win32WindowSurfaceAlpha* win32_window_surface_alpha(HWND hwnd) {
  return static_cast<Win32WindowSurfaceAlpha*>(GetPropW(hwnd, win32_surface_alpha_property));
}

} // namespace cgpui
