#include "win32_window_internal.hpp"

namespace cgpui {

PlatformWindowChromeState Win32Window::apply_window_chrome(
    WindowChromeOptions options) {
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
      .extended_style = win32_window_extended_style_for(options),
  };

  if (hwnd_ != nullptr) {
    if (display_command_state_.fullscreen) {
      display_command_state_.windowed_style = chrome_state_.style;
      display_command_state_.windowed_extended_style =
          chrome_state_.extended_style;
      return chrome_state_.platform;
    }
    SetWindowLongPtrW(hwnd_, GWL_STYLE, static_cast<LONG_PTR>(chrome_state_.style));
    SetWindowLongPtrW(
        hwnd_,
        GWL_EXSTYLE,
        static_cast<LONG_PTR>(chrome_state_.extended_style));
    if (options.transparent_background) {
      SetLayeredWindowAttributes(hwnd_, 0, 255, LWA_ALPHA);
    }
    SetWindowPos(
        hwnd_,
        nullptr,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE |
            SWP_FRAMECHANGED);
  }

  return chrome_state_.platform;
}

} // namespace cgpui
