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

DWORD win32_window_style_for(const WindowChromeOptions& chrome) {
  DWORD style = chrome.decorations ? WS_OVERLAPPEDWINDOW : WS_POPUP;
  // A client title bar keeps WS_CAPTION: WM_NCCALCSIZE removes the caption
  // area, and the style keeps snap, the system menu and min/max animations.
  if (!chrome.titlebar_visible &&
      chrome.title_bar != WindowTitleBarMode::client) {
    style &= ~WS_CAPTION;
  }
  if (!chrome.resizable) {
    style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
  }
  return style;
}

} // namespace cgpui
