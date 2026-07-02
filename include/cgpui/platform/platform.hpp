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

enum class NativeMenuItemKind {
  command,
  separator,
  submenu,
};

struct NativeMenuAccelerator {
  std::uint32_t key_code = 0;
  KeyAction action = KeyAction::pressed;
  KeyboardModifiers modifiers;
};

struct NativeMenuItem {
  NativeMenuItemKind kind = NativeMenuItemKind::command;
  std::string title;
  std::string action_name;
  std::optional<NativeMenuAccelerator> accelerator;
  bool enabled = true;
  bool checked = false;
  std::vector<NativeMenuItem> children;
};

struct NativeMenuModel {
  std::vector<NativeMenuItem> items;
};

struct PlatformMenuInstallationResult {
  bool supported = false;
  std::string backend;
  std::size_t menu_count = 0;
  std::size_t item_count = 0;
  std::size_t accelerator_count = 0;
};

struct NativeMenuInstallation {
  NativeMenuModel model;
  PlatformMenuInstallationResult platform;
};

enum class NativeFileDialogKind {
  open_file,
  open_files,
  save_file,
};

struct NativeFileDialogFilter {
  std::string name;
  std::vector<std::string> extensions;
};

struct NativeFileDialogOptions {
  NativeFileDialogKind kind = NativeFileDialogKind::open_file;
  std::string title;
  std::string default_directory;
  std::string suggested_name;
  std::vector<NativeFileDialogFilter> filters;
};

struct NativeFileDialogResult {
  bool supported = false;
  bool accepted = false;
  std::string backend;
  NativeFileDialogKind kind = NativeFileDialogKind::open_file;
  std::vector<std::string> paths;
  std::string error_message;
  std::size_t filter_count = 0;
};

struct PlatformWindowChromeState {
  bool supported = false;
  std::string backend;
  WindowChromeOptions requested;
  WindowChromeOptions applied;
  std::string reason;
};

enum class PlatformDiagnosticKind {
  unknown,
  clipboard,
  drag_drop,
  ime,
  accessibility,
  window_lifecycle,
  menu,
  file_dialog,
  window_chrome,
};

struct PlatformDiagnosticEvent {
  PlatformDiagnosticKind kind = PlatformDiagnosticKind::unknown;
  EventKind event_kind = EventKind::unknown;
  std::string backend;
  std::string operation;
  bool supported = true;
  bool succeeded = true;
  std::size_t value_count = 0;
  int sequence = 0;
};

[[nodiscard]] std::size_t native_menu_item_count(
    const NativeMenuModel& model);
[[nodiscard]] std::size_t native_menu_accelerator_count(
    const NativeMenuModel& model);

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
  virtual PlatformWindowChromeState apply_window_chrome(
      WindowChromeOptions options);
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
  virtual PlatformMenuInstallationResult install_native_menu(
      NativeMenuModel menu);
  virtual NativeFileDialogResult show_native_file_dialog(
      NativeFileDialogOptions options);

  virtual int run() = 0;
  virtual void quit() = 0;
};

Result<std::unique_ptr<PlatformApplication>> create_platform_application();

} // namespace cgpui
