#include "win32_window_backdrop_internal.hpp"
#include "win32_system_appearance_internal.hpp"

#include <dwmapi.h>

namespace cgpui {
namespace {

BOOL CALLBACK reevaluate_window(HWND hwnd, LPARAM lparam) {
  auto* backdrop =
      static_cast<Win32WindowBackdrop*>(GetPropW(hwnd, win32_window_backdrop_property));
  if (backdrop != nullptr) {
    (void)backdrop->reevaluate(
        *reinterpret_cast<const SystemAppearance*>(lparam));
  }
  return TRUE;
}

} // namespace

Win32WindowBackdrop::~Win32WindowBackdrop() { detach(); }

PlatformWindowBackdropState Win32WindowBackdrop::apply(
    HWND hwnd,
    WindowBackdrop backdrop,
    bool transparent_surface,
    bool layered,
    bool client_title_bar) {
  if (hwnd != hwnd_) {
    detach();
    hwnd_ = hwnd;
    if (hwnd_ != nullptr) SetPropW(hwnd_, win32_window_backdrop_property, this);
  }
  requested_ = backdrop;
  transparent_surface_ = transparent_surface;
  layered_ = layered;
  client_title_bar_ = client_title_bar;
  return reevaluate(win32_read_system_appearance());
}

PlatformWindowBackdropState Win32WindowBackdrop::reevaluate(
    const SystemAppearance& appearance) {
  return apply_plan(win32_plan_window_backdrop(Win32BackdropInputs{
      .requested = requested_,
      .transparent_surface = transparent_surface_,
      .os_build = win32_os_build_number(),
      .layered = layered_,
      .transparency_effects_enabled = appearance.transparency_effects_enabled,
      .client_title_bar = client_title_bar_,
  }));
}

PlatformWindowBackdropState Win32WindowBackdrop::apply_plan(
    const Win32BackdropPlan& plan) {
  state_ = plan.state;
  if (hwnd_ == nullptr) {
    state_.applied = WindowBackdrop::none;
    state_.title_bar = WindowBackdrop::none;
    state_.reason = "window has no HWND";
    return state_;
  }
  if (!plan.touch_dwm) return state_;
  const DWORD type = plan.backdrop_type;
  const HRESULT set = DwmSetWindowAttribute(
      hwnd_, win32_system_backdrop_attribute, &type, sizeof(type));
  // Margins of -1 extend the frame (and its backdrop) over the whole client
  // area; zero margins restore the normal opaque client area.
  const int extent = plan.extend_frame ? -1 : 0;
  const MARGINS margins{extent, extent, extent, extent};
  const HRESULT extended = DwmExtendFrameIntoClientArea(hwnd_, &margins);
  frame_extended_ = plan.extend_frame && SUCCEEDED(extended);
  // Erase again so the redirection surface DWM now shows is black (alpha 0).
  RedrawWindow(hwnd_, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE);
  if (FAILED(set) || FAILED(extended)) {
    state_.applied = WindowBackdrop::none;
    if (FAILED(set)) state_.title_bar = WindowBackdrop::none;
    state_.reason = FAILED(set)
        ? "DwmSetWindowAttribute(DWMWA_SYSTEMBACKDROP_TYPE) failed"
        : "DwmExtendFrameIntoClientArea failed";
  }
  return state_;
}

void Win32WindowBackdrop::detach() {
  if (hwnd_ != nullptr && IsWindow(hwnd_)) {
    RemovePropW(hwnd_, win32_window_backdrop_property);
  }
  hwnd_ = nullptr;
}

const PlatformWindowBackdropState& Win32WindowBackdrop::state() const {
  return state_;
}

void win32_reevaluate_thread_window_backdrops(
    const SystemAppearance& appearance) {
  EnumThreadWindows(
      GetCurrentThreadId(), reevaluate_window,
      reinterpret_cast<LPARAM>(&appearance));
}

} // namespace cgpui
