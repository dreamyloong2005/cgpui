#include "win32_window_internal.hpp"

namespace cgpui {

PlatformWindowChromeState Win32Window::apply_window_chrome(
    WindowChromeOptions options) {
  if (!surface_alpha_.accepts_chrome(options)) {
    auto rejected = chrome_state_.platform;
    rejected.requested = options;
    rejected.reason = "transparent DX12 ordinary windows require decorated system title bars";
    return rejected;
  }
  surface_alpha_.set_chrome(options);
  const WindowTitleBarMode previous_title_bar =
      chrome_state_.platform.applied.title_bar;
  chrome_state_ = Win32WindowChromeState{
      .platform =
          PlatformWindowChromeState{
              .supported = true,
              .decoration_control_supported = true,
              .transparency_supported = true,
              .backend = "win32",
              .requested = options,
              .applied = options,
          },
      .style = win32_window_style_for(options),
  };

  if (hwnd_ != nullptr) {
    title_bar_.dpi = GetDpiForWindow(hwnd_);
    if (display_command_state_.fullscreen) {
      display_command_state_.windowed_style = chrome_state_.style;
      return chrome_state_.platform;
    }
    // Chrome sets no extended style: transparent_background only asks the
    // renderer for a transparent surface (docs/window-backdrop.md, Windows).
    SetWindowLongPtrW(hwnd_, GWL_STYLE, static_cast<LONG_PTR>(chrome_state_.style));
    SetWindowPos(
        hwnd_,
        nullptr,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE |
            SWP_FRAMECHANGED);
    const PlatformWindowBackdropState backdrop = backdrop_.state();
    if (options.title_bar != previous_title_bar &&
        backdrop.requested != WindowBackdrop::none) {
      (void)apply_window_backdrop(backdrop.requested, backdrop.transparent_surface);
    }
  }

  return chrome_state_.platform;
}

} // namespace cgpui
