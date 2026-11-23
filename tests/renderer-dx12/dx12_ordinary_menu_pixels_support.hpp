#pragma once

#include "dx12_popup_test_support.hpp"
#include <dwmapi.h>
#include <algorithm>
#include <vector>

namespace cgpui_test {

inline bool own_menu_target(HWND hwnd) {
  DWORD process = 0;
  const DWORD thread = GetWindowThreadProcessId(hwnd, &process);
  return thread == GetCurrentThreadId() && process == GetCurrentProcessId();
}

struct MenuPixels {
  RECT rectangle{};
  std::vector<DWORD> pixels;

  bool capture(HWND hwnd) {
    pixels.clear();
    MENUBARINFO bar{sizeof(bar)};
    RECT window{};
    if (!own_menu_target(hwnd) || !IsWindowVisible(hwnd) || !GetWindowRect(hwnd, &window) ||
        !GetMenuBarInfo(hwnd, OBJID_MENU, 0, &bar)) return false;
    rectangle = bar.rcBar;
    const LONG width = rectangle.right - rectangle.left, height = rectangle.bottom - rectangle.top;
    if (width <= 0 || height <= 0 || rectangle.left < window.left || rectangle.top < window.top ||
        rectangle.right > window.right || rectangle.bottom > window.bottom) return false;
    // Capture only the complete menu-bar rectangle of this process's window.
    struct CaptureResources {
      HDC screen = GetDC(nullptr), memory = nullptr;
      HBITMAP bitmap = nullptr;
      HGDIOBJ previous = nullptr;
      ~CaptureResources() {
        if (previous && previous != HGDI_ERROR) SelectObject(memory, previous);
        if (bitmap) DeleteObject(bitmap);
        if (memory) DeleteDC(memory);
        if (screen) ReleaseDC(nullptr, screen);
      }
    } resources;
    if (!resources.screen || !(resources.memory = CreateCompatibleDC(resources.screen))) return false;
    BITMAPINFO info{};
    info.bmiHeader = {sizeof(BITMAPINFOHEADER), width, -height, 1, 32, BI_RGB};
    void* data = nullptr;
    resources.bitmap = CreateDIBSection(resources.screen, &info, DIB_RGB_COLORS, &data, nullptr, 0);
    if (!resources.bitmap || !data) return false;
    resources.previous = SelectObject(resources.memory, resources.bitmap);
    if (!resources.previous || resources.previous == HGDI_ERROR) return false;
    if (!BitBlt(resources.memory, 0, 0, width, height, resources.screen,
                rectangle.left, rectangle.top, SRCCOPY | CAPTUREBLT) || !GdiFlush()) return false;
    const auto count = std::size_t(width) * std::size_t(height);
    pixels.assign(static_cast<const DWORD*>(data), static_cast<const DWORD*>(data) + count);
    return true;
  }

  std::size_t different(const MenuPixels& other) const {
    if (pixels.empty() || other.pixels.empty() || pixels.size() != other.pixels.size() ||
        rectangle.right - rectangle.left != other.rectangle.right - other.rectangle.left ||
        rectangle.bottom - rectangle.top != other.rectangle.bottom - other.rectangle.top)
      return std::max<std::size_t>({1, pixels.size(), other.pixels.size()});
    std::size_t changed = 0;
    for (std::size_t i = 0; i < pixels.size(); ++i)
      changed += (pixels[i] & 0xffffff) != (other.pixels[i] & 0xffffff);
    return changed;
  }
};

struct MenuFocus {
  bool active = false, foreground = false, focused = false, bar = false, item = false;
  friend bool operator==(const MenuFocus&, const MenuFocus&) = default;
};

inline MenuFocus menu_focus(HWND hwnd) {
  MENUBARINFO bar{sizeof(bar)};
  GetMenuBarInfo(hwnd, OBJID_MENU, 0, &bar);
  return {GetActiveWindow() == hwnd, GetForegroundWindow() == hwnd, GetFocus() == hwnd,
      bar.fBarFocused != 0, bar.fFocused != 0};
}

inline void print_focus(const MenuFocus& focus) {
  std::cout << focus.active << '/' << focus.foreground << '/' << focus.focused << '/'
            << focus.bar << '/' << focus.item;
}

inline bool stable_menu(HWND hwnd, MenuPixels& pixels, MenuFocus& focus) {
  if (!pixels.capture(hwnd)) return false;
  focus = menu_focus(hwnd);
  unsigned stable = 0;
  const ULONGLONG end = GetTickCount64() + 2000;
  do {
    pump_messages_for(100);
    DwmFlush();
    MenuPixels next;
    if (!next.capture(hwnd)) return false;
    const auto next_focus = menu_focus(hwnd);
    stable = pixels.different(next) == 0 && next_focus == focus ? stable + 1 : 0;
    pixels = std::move(next);
    focus = next_focus;
  } while (stable != 3 && GetTickCount64() < end);
  return stable == 3;
}

} // namespace cgpui_test
