#pragma once

#include "win32_internal.hpp"

#include <memory>
#include <optional>
#include <string_view>

namespace cgpui {

class Win32Window final
    : public PlatformWindow,
      public Win32OleDropTargetOwner,
      public Win32WindowMessageTarget {
 public:
  Win32Window(
      HINSTANCE instance,
      PlatformEventCallback callback,
      WindowState state);
  ~Win32Window() override;

  void attach(HWND hwnd) override;
  void detach() override;
  [[nodiscard]] NativeSurfaceHandle native_surface() const override;
  [[nodiscard]] WindowState state() const override;
  void request_redraw() override;
  void request_close() override;
  void set_title(std::string_view title) override;
  void set_cursor(CursorShape cursor_shape) override;
  void set_ime_text_input_placement(
      std::optional<ImeTextInputPlacement> placement) override;
  PlatformWindowChromeState apply_window_chrome(
      WindowChromeOptions options) override;
  void update_accessibility_tree(
      PlatformAccessibilityTreeUpdate update) override;

  void update_size() override;
  void update_size_for_dpi(float dpi);
  void dpi_changed(WPARAM wparam, LPARAM lparam) override;
  void focus_changed(bool focused) override;
  void ime_start_composition() override;
  void close_requested() override;
  void redraw_requested() override;
  void wakeup_requested();
  void pointer_moved(LPARAM lparam) override;
  void pointer_button(
      MouseButton button,
      bool pressed,
      LPARAM lparam) override;
  void pointer_scrolled(WPARAM wparam, LPARAM lparam) override;
  void key_event(WPARAM wparam, KeyAction action) override;
  void text_input(WPARAM wparam) override;
  void drag_entered(const Win32TestDragDropPayload* payload) override;
  void drag_updated(const Win32TestDragDropPayload* payload) override;
  void drag_dropped(const Win32TestDragDropPayload* payload) override;
  void drag_exited(const Win32TestDragDropPayload* payload) override;

  void ole_drag_entered(
      IDataObject* data_object,
      POINTL point,
      DWORD key_state,
      DWORD* effect) override;
  void ole_drag_updated(
      POINTL point,
      DWORD key_state,
      DWORD* effect) override;
  void ole_drag_exited() override;
  void ole_drag_dropped(
      IDataObject* data_object,
      POINTL point,
      DWORD key_state,
      DWORD* effect) override;

 private:
  void register_drop_target(HWND hwnd);
  void revoke_drop_target();
  Point client_position_from_screen(POINTL point) const;
  void apply_ime_text_input_placement();

  HINSTANCE instance_ = nullptr;
  HWND hwnd_ = nullptr;
  HCURSOR current_cursor_ = nullptr;
  PlatformEventCallback callback_;
  WindowState state_;
  Win32WindowChromeState chrome_state_;
  Win32UiaAccessibilityAdapter uia_accessibility_;
  std::unique_ptr<Win32OleDropTarget> ole_drop_target_;
  Win32OleDropTargetRegistrationState ole_drop_target_registration_;
  Point last_ole_drag_position_{};
  DragDropPayload last_ole_drag_payload_;
};

} // namespace cgpui
