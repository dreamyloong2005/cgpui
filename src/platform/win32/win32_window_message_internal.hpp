#pragma once

#include "cgpui/platform/platform.hpp"
#include "win32_drag_drop_internal.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace cgpui {

inline constexpr WPARAM win32_application_close_wparam = 1;

struct Win32WindowChromeState {
  PlatformWindowChromeState platform;
  DWORD style = WS_OVERLAPPEDWINDOW;
  DWORD extended_style = 0;
};

class Win32WindowMessageTarget {
 public:
  virtual ~Win32WindowMessageTarget() = default;
  virtual void attach(HWND hwnd) = 0;
  virtual void detach() = 0;
  virtual void update_size() = 0;
  virtual void position_changed() = 0;
  virtual void dpi_changed(WPARAM wparam, LPARAM lparam) = 0;
  virtual void activation_changed(bool active) = 0;
  virtual void focus_changed(bool focused) = 0;
  virtual void ime_start_composition() = 0;
  virtual void ime_composition(LPARAM lparam) = 0;
  virtual void ime_end_composition() = 0;
  virtual void close_requested(WindowCloseRequestSource source) = 0;
  virtual void redraw_requested() = 0;
  virtual void pointer_moved(LPARAM lparam) = 0;
  virtual void pointer_button(
      MouseButton button,
      bool pressed,
      std::uint8_t click_count,
      LPARAM lparam) = 0;
  virtual void pointer_scrolled(
      Point delta,
      bool precise,
      LPARAM lparam) = 0;
  virtual void key_event(KeyboardKey event) = 0;
  virtual void text_input(WPARAM wparam) = 0;
  virtual void drag_entered(const Win32TestDragDropPayload* payload) = 0;
  virtual void drag_updated(const Win32TestDragDropPayload* payload) = 0;
  virtual void drag_dropped(const Win32TestDragDropPayload* payload) = 0;
  virtual void drag_exited(const Win32TestDragDropPayload* payload) = 0;
};

} // namespace cgpui
