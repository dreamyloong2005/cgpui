#pragma once

#include "cgpui/core/event_platform.hpp"
#include "cgpui/core/window.hpp"
#include "cgpui/platform/native_surface.hpp"
#include "cgpui/platform/platform_accessibility.hpp"
#include "cgpui/platform/platform_window_chrome.hpp"
#include "cgpui/platform/platform_window_close.hpp"
#include "cgpui/platform/platform_window_lifecycle.hpp"
#include "cgpui/platform/platform_window_position.hpp"

#include <functional>
#include <optional>
#include <string_view>

namespace cgpui {

class PlatformWindow {
 public:
  virtual ~PlatformWindow() = default;

  [[nodiscard]] virtual NativeSurfaceHandle native_surface() const = 0;
  [[nodiscard]] virtual WindowState state() const = 0;
  [[nodiscard]] virtual PlatformWindowLifecycleState lifecycle_state() const;
  virtual bool request_display_state(PlatformWindowDisplayState display_state);
  [[nodiscard]] virtual PlatformWindowPositionState position_state() const;
  virtual bool request_position(Point position);
  [[nodiscard]] virtual PlatformWindowCloseState close_request_state() const;
  virtual bool resolve_close_request(PlatformWindowCloseResolution resolution);
  virtual void request_redraw() = 0;
  virtual void request_close() = 0;
  virtual void set_title(std::string_view title) = 0;
  virtual void set_cursor(CursorShape cursor_shape) = 0;
  virtual void set_ime_text_input_placement(
      std::optional<ImeTextInputPlacement> placement) = 0;
  virtual PlatformWindowChromeState apply_window_chrome(
      WindowChromeOptions options);
  virtual void update_accessibility_tree(
      PlatformAccessibilityTreeUpdate update);
};

using PlatformEventCallback = std::function<void(const PlatformEvent&)>;

} // namespace cgpui
