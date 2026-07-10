#pragma once

class RegisteredWaylandWindow final : public PlatformWindow {
 public:
  RegisteredWaylandWindow(
      WaylandWindowPtr window,
      WaylandWindowUnregisterCallback unregister,
      WaylandWindowCursorCallback set_cursor,
      WaylandWindowImePlacementCallback set_ime_placement);
  ~RegisteredWaylandWindow() override;

  [[nodiscard]] NativeSurfaceHandle native_surface() const override;
  [[nodiscard]] WindowState state() const override;
  [[nodiscard]] PlatformWindowLifecycleState lifecycle_state() const override;
  [[nodiscard]] PlatformWindowCloseState close_request_state() const override;
  bool resolve_close_request(PlatformWindowCloseResolution resolution) override;

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

 private:
  WaylandWindowPtr window_;
  WaylandWindowUnregisterCallback unregister_;
  WaylandWindowCursorCallback set_cursor_;
  WaylandWindowImePlacementCallback set_ime_placement_;
};
