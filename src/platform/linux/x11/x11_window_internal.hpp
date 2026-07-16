#pragma once

#include "x11_internal.hpp"
#include "x11_drag_drop_internal.hpp"
#include "../../platform_window_close_internal.hpp"

#include <functional>

namespace cgpui {

struct X11KeyboardState;

class X11Window final : public PlatformWindow {
 public:
  static Result<std::unique_ptr<X11Window>> create(
      xcb_connection_t* connection,
      xcb_screen_t* screen,
      const X11Atoms& atoms,
      DpiScale scale,
      std::shared_ptr<X11KeyboardState> keyboard,
      xcb_cursor_context_t* cursor_context,
      xcb_window_t parent,
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback,
      std::function<void(X11Window*)> unregister);
  ~X11Window() override;

  [[nodiscard]] NativeSurfaceHandle native_surface() const override;
  [[nodiscard]] WindowState state() const override;
  [[nodiscard]] PlatformWindowLifecycleState lifecycle_state() const override;
  bool request_display_state(PlatformWindowDisplayState display_state) override;
  [[nodiscard]] PlatformWindowPositionState position_state() const override;
  bool request_position(Point position) override;
  [[nodiscard]] PlatformWindowCloseState close_request_state() const override;
  bool resolve_close_request(PlatformWindowCloseResolution resolution) override;
  void request_redraw() override;
  void request_close() override;
  void set_title(std::string_view title) override;
  void set_cursor(CursorShape cursor_shape) override;
  [[nodiscard]] PlatformPointerCaptureState
  pointer_capture_state() const override;
  void set_pointer_capture(bool captured) override;
  void set_ime_text_input_placement(
      std::optional<ImeTextInputPlacement> placement) override;
  PlatformWindowChromeState apply_window_chrome(
      WindowChromeOptions options) override;

  [[nodiscard]] bool owns_event(const xcb_generic_event_t& event) const;
  void handle_event(const xcb_generic_event_t& event);
  void wakeup_requested();
  void handle_pointer_event(const xcb_generic_event_t& event);
  void handle_keyboard_event(const xcb_generic_event_t& event);
  [[nodiscard]] bool handle_drag_client_message(
      const xcb_client_message_event_t& message);
  void handle_drag_selection_notify(
      const xcb_selection_notify_event_t& notify);

 private:
  X11Window(
      xcb_connection_t* connection,
      xcb_screen_t* screen,
      X11Atoms atoms,
      DpiScale scale,
      std::shared_ptr<X11KeyboardState> keyboard,
      xcb_cursor_context_t* cursor_context,
      PlatformEventCallback callback,
      std::function<void(X11Window*)> unregister);
  Result<void> initialize(
      xcb_window_t parent,
      const WindowDescriptor& descriptor);
  void begin_close_request(WindowCloseRequestSource source);
  void destroy_native_window();
  void send_net_wm_state(std::uint32_t action, xcb_atom_t first, xcb_atom_t second);
  void send_drag_status();
  void finish_drag(bool accepted);
  void reset_drag();

  xcb_connection_t* connection_ = nullptr;
  xcb_screen_t* screen_ = nullptr;
  X11Atoms atoms_;
  std::shared_ptr<X11KeyboardState> keyboard_;
  xcb_cursor_context_t* cursor_context_ = nullptr;
  xcb_cursor_t cursor_ = XCB_CURSOR_NONE;
  xcb_window_t window_ = XCB_WINDOW_NONE;
  PlatformEventCallback callback_;
  std::function<void(X11Window*)> unregister_;
  WindowState state_;
  PlatformWindowCloseController close_controller_;
  PlatformWindowDisplayState display_state_ =
      PlatformWindowDisplayState::normal;
  std::optional<Point> position_;
  WindowChromeOptions chrome_;
  CursorShape cursor_shape_ = CursorShape::default_arrow;
  bool pointer_captured_ = false;
  bool configured_ = false;
  bool active_ = false;
  bool focused_ = false;
  X11DragDropState drag_;
};

}  // namespace cgpui
