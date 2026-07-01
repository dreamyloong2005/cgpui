#pragma once

#include "cgpui/core/error.hpp"
#include "cgpui/core/events.hpp"
#include "cgpui/core/window.hpp"
#include "cgpui/platform/native_surface.hpp"
#include "cgpui/platform/target.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string_view>

namespace cgpui {

class FontDatabase;

class PlatformWindow {
 public:
  virtual ~PlatformWindow() = default;

  [[nodiscard]] virtual NativeSurfaceHandle native_surface() const = 0;
  [[nodiscard]] virtual WindowState state() const = 0;
  virtual void request_redraw() = 0;
  virtual void request_close() = 0;
  virtual void set_title(std::string_view title) = 0;
  virtual void set_cursor(CursorShape cursor_shape) = 0;
  virtual void set_ime_text_input_placement(
      std::optional<ImeTextInputPlacement> placement) = 0;
};

using PlatformEventCallback = std::function<void(const PlatformEvent&)>;

class PlatformApplication {
 public:
  virtual ~PlatformApplication() = default;

  virtual Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) = 0;
  [[nodiscard]] virtual FontDatabase discover_fonts() const;

  virtual int run() = 0;
  virtual void quit() = 0;
};

Result<std::unique_ptr<PlatformApplication>> create_platform_application();

} // namespace cgpui
