#include "win32_internal.hpp"

namespace cgpui {

KeyboardModifiers current_modifiers() {
  return KeyboardModifiers{
      .shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0,
      .control = (GetKeyState(VK_CONTROL) & 0x8000) != 0,
      .alt = (GetKeyState(VK_MENU) & 0x8000) != 0,
      .super = (GetKeyState(VK_LWIN) & 0x8000) != 0 ||
          (GetKeyState(VK_RWIN) & 0x8000) != 0,
  };
}

const wchar_t* cursor_id_for(CursorShape cursor_shape) {
  switch (cursor_shape) {
    case CursorShape::pointing_hand:
      return MAKEINTRESOURCEW(32649);
    case CursorShape::text:
      return MAKEINTRESOURCEW(32513);
    case CursorShape::crosshair:
      return MAKEINTRESOURCEW(32515);
    case CursorShape::resize_left_right:
      return MAKEINTRESOURCEW(32644);
    case CursorShape::resize_up_down:
      return MAKEINTRESOURCEW(32645);
    case CursorShape::not_allowed:
      return MAKEINTRESOURCEW(32648);
    case CursorShape::default_arrow:
      return MAKEINTRESOURCEW(32512);
  }
  return MAKEINTRESOURCEW(32512);
}

DWORD win32_window_style_for(const WindowChromeOptions& chrome) {
  DWORD style = chrome.decorations ? WS_OVERLAPPEDWINDOW : WS_POPUP;
  if (!chrome.titlebar_visible) {
    style &= ~WS_CAPTION;
  }
  if (!chrome.resizable) {
    style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
  }
  return style;
}

DWORD win32_window_extended_style_for(const WindowChromeOptions& chrome) {
  DWORD style = 0;
  if (chrome.transparent_background) {
    style |= WS_EX_LAYERED;
  }
  return style;
}

} // namespace cgpui
