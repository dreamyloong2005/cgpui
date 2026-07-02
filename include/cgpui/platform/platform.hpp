#pragma once

#include "cgpui/core/error.hpp"
#include "cgpui/core/events.hpp"
#include "cgpui/core/window.hpp"
#include "cgpui/platform/native_surface.hpp"
#include "cgpui/platform/target.hpp"
#include "cgpui/ui/text.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

enum class PlatformAccessibilityRole {
  generic,
  label,
  button,
  text,
  text_input,
};

struct PlatformAccessibilityNodeUpdate {
  std::uint64_t element_id = 0;
  std::optional<std::uint64_t> parent_element_id;
  PlatformAccessibilityRole role = PlatformAccessibilityRole::generic;
  std::string name;
  std::string text;
  bool enabled = true;
  bool focusable = false;
  bool focused = false;
  std::optional<Rect> bounds;
  std::size_t child_count = 0;
};

struct PlatformAccessibilityTreeUpdate {
  std::uint64_t root_element_id = 0;
  std::size_t node_count = 0;
  std::size_t focused_node_count = 0;
  std::vector<PlatformAccessibilityNodeUpdate> nodes;
};

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
  virtual void update_accessibility_tree(
      PlatformAccessibilityTreeUpdate update);
};

using PlatformEventCallback = std::function<void(const PlatformEvent&)>;

class PlatformApplication {
 public:
  virtual ~PlatformApplication() = default;

  virtual Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) = 0;
  [[nodiscard]] virtual std::vector<FontFaceDescriptor> discover_font_records()
      const;
  [[nodiscard]] virtual FontDatabase discover_fonts() const;
  virtual void request_wakeup();

  virtual int run() = 0;
  virtual void quit() = 0;
};

Result<std::unique_ptr<PlatformApplication>> create_platform_application();

} // namespace cgpui
