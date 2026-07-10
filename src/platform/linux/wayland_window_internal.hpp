#pragma once

#include "wayland_internal.hpp"
#include "wayland_window_state.hpp"

namespace cgpui {

class WaylandWindow final : public PlatformWindow {
 public:
  static Result<std::unique_ptr<WaylandWindow>> create(
      wl_display* display,
      wl_compositor* compositor,
      xdg_wm_base* shell,
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback,
      bool text_input_available);

  ~WaylandWindow() override;

  [[nodiscard]] NativeSurfaceHandle native_surface() const override;
  [[nodiscard]] wl_surface* surface() const;
  [[nodiscard]] WindowState state() const override;
  [[nodiscard]] PlatformWindowLifecycleState lifecycle_state() const override;

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

  [[nodiscard]] CursorShape cursor_shape() const;
  [[nodiscard]] bool configured() const;

  void pointer_moved(Point position);
  void pointer_button(MouseButton button, bool pressed, Point position);
  void pointer_scrolled(Point delta, Point position);
  void drag_entered(
      Point position,
      DragDropPayload payload,
      DragDropAction action);
  void drag_updated(
      Point position,
      DragDropPayload payload,
      DragDropAction action);
  void drag_dropped(
      Point position,
      DragDropPayload payload,
      DragDropAction action);
  void drag_exited(Point position);
  void wakeup_requested();
  void keyboard_key(
      std::uint32_t key,
      KeyAction action,
      KeyboardModifiers modifiers);
  void text_input(std::string text, KeyboardModifiers modifiers);
  void text_input_entered();
  void text_input_left(KeyboardModifiers modifiers);
  void text_input_preedit(
      std::string text,
      std::int32_t cursor_begin,
      std::int32_t cursor_end,
      KeyboardModifiers modifiers,
      std::uint32_t serial);
  void text_input_commit(
      std::string text,
      KeyboardModifiers modifiers,
      std::uint32_t serial);
  void text_input_delete_surrounding(
      std::uint32_t before_length,
      std::uint32_t after_length,
      KeyboardModifiers modifiers,
      std::uint32_t serial);
  void text_input_surrounding_text(
      std::string text,
      std::int32_t cursor,
      std::int32_t anchor);
  void text_input_content_type(std::uint32_t hint, std::uint32_t purpose);
  void set_text_input_available(bool available);
  void focus_changed(bool focused);

 private:
  WaylandWindow(
      wl_display* display,
      PlatformEventCallback callback,
      WindowState state);

  Result<void> initialize(
      wl_compositor* compositor,
      xdg_wm_base* shell,
      const WindowDescriptor& descriptor);
#include "wayland_window_configure_internal.hpp"
  void sync_text_input_state();

  wl_display* display_ = nullptr;
  wl_surface* surface_ = nullptr;
  xdg_surface* xdg_surface_ = nullptr;
  xdg_toplevel* toplevel_ = nullptr;
  PlatformEventCallback callback_;
  WindowState state_;
  WaylandTextInputState text_input_state_;
  WaylandAtspiAccessibilityAdapterPtr atspi_accessibility_ =
      create_wayland_atspi_accessibility_adapter();
  CursorShape cursor_shape_ = CursorShape::default_arrow;
  WaylandWindowChromeState chrome_state_;
  WaylandXdgConfigureState pending_configure_;
  bool configured_ = false;
  bool resize_pending_surface_configure_ = false;
  bool focused_ = false;
};

#include "wayland_registered_window_internal.hpp"

} // namespace cgpui
