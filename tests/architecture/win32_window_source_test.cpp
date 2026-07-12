#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

std::string read_source(const char* path) {
  std::ifstream source(path);
  if (!source) {
    source.open((std::string("../../../../") + path).c_str());
  }
  if (!source) {
    return {};
  }

  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  return static_cast<std::size_t>(
      std::count(text.begin(), text.end(), '\n'));
}

std::string read_win32_source() {
  const std::vector<const char*> win32_files{
      "src/platform/win32/win32_internal.hpp",
      "src/platform/win32/win32_accessibility_internal.hpp",
      "src/platform/win32/win32_drag_drop_internal.hpp",
      "src/platform/win32/win32_native_menu_accelerator_internal.hpp",
      "src/platform/win32/win32_native_menu_accelerator_table_internal.hpp",
      "src/platform/win32/win32_native_menu_command_internal.hpp",
      "src/platform/win32/win32_native_internal.hpp",
      "src/platform/win32/win32_native_menu_tree_internal.hpp",
      "src/platform/win32/win32_window_message_internal.hpp",
      "src/platform/win32/win32_window_internal.hpp",
      "src/platform/win32/win32_window_display_internal.hpp",
      "src/platform/win32/win32_uia_navigation_internal.hpp",
      "src/platform/win32/win32_uia_provider_internal.hpp",
      "src/platform/win32/win32_uia_provider_object_internal.hpp",
      "src/platform/win32/win32_accessibility.cpp",
      "src/platform/win32/win32_helpers.cpp",
      "src/platform/win32/win32_string.cpp",
      "src/platform/win32/win32_drag_drop_helpers.cpp",
      "src/platform/win32/win32_drag_drop_payload.cpp",
      "src/platform/win32/win32_drag_drop_ole_payload.cpp",
      "src/platform/win32/win32_input_helpers.cpp",
      "src/platform/win32/win32_input_coordinates_internal.hpp",
      "src/platform/win32/win32_input_coordinates.cpp",
      "src/platform/win32/win32_cursor_internal.hpp",
      "src/platform/win32/win32_cursor.cpp",
      "src/platform/win32/win32_dead_key_internal.hpp",
      "src/platform/win32/win32_dead_key.cpp",
      "src/platform/win32/win32_keyboard_key_internal.hpp",
      "src/platform/win32/win32_keyboard_key.cpp",
      "src/platform/win32/win32_message_dialog_internal.hpp",
      "src/platform/win32/win32_message_dialog.cpp",
      "src/platform/win32/win32_font_discovery.cpp",
      "src/platform/win32/win32_file_dialog_internal.hpp",
      "src/platform/win32/win32_file_dialog.cpp",
      "src/platform/win32/win32_file_dialog_save.cpp",
      "src/platform/win32/win32_application.cpp",
      "src/platform/win32/win32_native_menu_accelerator.cpp",
      "src/platform/win32/win32_native_menu_accelerator_table.cpp",
      "src/platform/win32/win32_native_menu_command.cpp",
      "src/platform/win32/win32_native.cpp",
      "src/platform/win32/win32_native_menu_tree.cpp",
      "src/platform/win32/win32_open_url_internal.hpp",
      "src/platform/win32/win32_open_url.cpp",
      "src/platform/win32/win32_ole_drop_target.cpp",
      "src/platform/win32/win32_ole_drop_source.cpp",
      "src/platform/win32/win32_pointer_button_internal.hpp",
      "src/platform/win32/win32_pointer_button.cpp",
      "src/platform/win32/win32_pointer_capture_internal.hpp",
      "src/platform/win32/win32_pointer_capture.cpp",
      "src/platform/win32/win32_pointer_scroll_internal.hpp",
      "src/platform/win32/win32_pointer_scroll.cpp",
      "src/platform/win32/win32_text_input_internal.hpp",
      "src/platform/win32/win32_text_input.cpp",
      "src/platform/win32/win32_uia_fragment.cpp",
      "src/platform/win32/win32_uia_navigation.cpp",
      "src/platform/win32/win32_uia_provider.cpp",
      "src/platform/win32/win32_window.cpp",
      "src/platform/win32/win32_window_accessibility.cpp",
      "src/platform/win32/win32_window_chrome.cpp",
      "src/platform/win32/win32_window_close.cpp",
      "src/platform/win32/win32_window_display.cpp",
      "src/platform/win32/win32_window_position.cpp",
      "src/platform/win32/win32_window_pointer_capture.cpp",
      "src/platform/win32/win32_window_pointer_events.cpp",
      "src/platform/win32/win32_window_size.cpp",
      "src/platform/win32/win32_window_cursor.cpp",
      "src/platform/win32/win32_window_drag_drop.cpp",
      "src/platform/win32/win32_window_events.cpp",
      "src/platform/win32/win32_window_native_menu.cpp",
      "src/platform/win32/win32_window_text.cpp",
      "src/platform/win32/win32_window_factory_internal.hpp",
      "src/platform/win32/win32_window_factory.cpp",
      "src/platform/win32/win32_window_ime.cpp",
      "src/platform/win32/win32_window_ime_placement.cpp",
      "src/platform/win32/win32_window_proc.cpp",
      "src/platform/win32/win32_window_proc_accessibility_internal.hpp",
      "src/platform/win32/win32_window_proc_accessibility.cpp",
      "src/platform/win32/win32_window_proc_command_internal.hpp",
      "src/platform/win32/win32_window_proc_command.cpp",
      "src/platform/win32/win32_window_proc_drag.cpp",
      "src/platform/win32/win32_window_proc_lifecycle.cpp",
      "src/platform/win32/win32_window_proc_pointer.cpp",
      "src/platform/win32/win32_window_proc_pointer_capture_internal.hpp",
      "src/platform/win32/win32_window_proc_pointer_capture.cpp",
      "src/platform/win32/win32_window_proc_cursor_internal.hpp",
      "src/platform/win32/win32_window_proc_cursor.cpp",
      "src/platform/win32/win32_window_proc_keyboard.cpp",
      "src/platform/win32/win32_window_proc_text.cpp",
  };

  std::string text;
  for (const char* path : win32_files) {
    const std::string source = read_source(path);
    if (source.empty()) {
      return {};
    }
    text += source;
    text += '\n';
  }
  return text;
}

std::string target_block(const std::string& xmake_text, const char* target_name) {
  const std::string target_marker =
      std::string("target(\"") + target_name + "\")";
  const auto target_position = xmake_text.find(target_marker);
  if (target_position == std::string::npos) {
    return {};
  }

  const auto next_target_position = xmake_text.find("\ntarget(\"", target_position + 1);
  const auto next_platform_position = xmake_text.find("\nif is_plat(", target_position + 1);
  auto end_position = std::string::npos;
  if (next_target_position != std::string::npos &&
      next_platform_position != std::string::npos) {
    end_position = std::min(next_target_position, next_platform_position);
  } else if (next_target_position != std::string::npos) {
    end_position = next_target_position;
  } else if (next_platform_position != std::string::npos) {
    end_position = next_platform_position;
  }

  return xmake_text.substr(
      target_position,
      end_position == std::string::npos
          ? std::string::npos
          : end_position - target_position);
}

std::string platform_branch(const std::string& target_text, const char* platform_name) {
  const std::string if_marker =
      std::string("if is_plat(\"") + platform_name + "\") then";
  const std::string elseif_marker =
      std::string("elseif is_plat(\"") + platform_name + "\") then";

  auto branch_position = target_text.find(if_marker);
  if (branch_position == std::string::npos) {
    branch_position = target_text.find(elseif_marker);
  }
  if (branch_position == std::string::npos) {
    return {};
  }

  auto end_position = target_text.find("\n    elseif is_plat(", branch_position + 1);
  const auto else_position = target_text.find("\n    else", branch_position + 1);
  const auto end_marker_position = target_text.find("\n    end", branch_position + 1);
  if (end_position == std::string::npos ||
      (else_position != std::string::npos && else_position < end_position)) {
    end_position = else_position;
  }
  if (end_position == std::string::npos ||
      (end_marker_position != std::string::npos && end_marker_position < end_position)) {
    end_position = end_marker_position;
  }

  return target_text.substr(
      branch_position,
      end_position == std::string::npos
          ? std::string::npos
          : end_position - branch_position);
}

} // namespace

int main() {
  const std::string win32_text = read_win32_source();
  if (win32_text.empty()) {
    return 2;
  }

  if (!contains(win32_text, "class Win32Window final")) {
    return 3;
  }
  if (!contains(win32_text, "Win32SurfaceHandle{.hinstance = instance_")) {
    return 4;
  }
  if (!contains(win32_text, "CreateWindowExW(")) {
    return 5;
  }
  if (!contains(win32_text, "PostMessageW(hwnd_, WM_CLOSE")) {
    return 6;
  }
  if (!contains(win32_text, "cgpui_wakeup_message") ||
      !contains(win32_text, "PostThreadMessageW") ||
      !contains(win32_text, "WindowWakeupRequested{}")) {
    return 55;
  }
  if (!contains(win32_text, "WM_MOUSEWHEEL")) {
    return 7;
  }
  if (!contains(win32_text, "WM_KEYDOWN") ||
      !contains(win32_text, "WM_KEYUP")) {
    return 8;
  }
  if (!contains(win32_text, "WM_DPICHANGED")) {
    return 31;
  }
  if (!contains(win32_text, "HIWORD(wparam)")) {
    return 32;
  }
  if (!contains(win32_text, "WM_SETFOCUS")) {
    return 37;
  }
  if (!contains(win32_text, "WM_KILLFOCUS")) {
    return 38;
  }
  if (!contains(win32_text, "WindowFocused{.focused = focused}")) {
    return 39;
  }
  if (!contains(win32_text, "WM_CHAR")) {
    return 44;
  }
  if (!contains(win32_text, "TextInput{")) {
    return 45;
  }
  if (!contains(win32_text, "GetKeyState(VK_SHIFT)")) {
    return 46;
  }
  if (!contains(win32_text, "Win32TestDragDropPayload") ||
      !contains(win32_text, "DragDropPayloadKind::text") ||
      !contains(win32_text, "DragDropPayloadKind::files")) {
    return 52;
  }
  if (!contains(win32_text, "Win32UiaAccessibilityAdapter") ||
      !contains(win32_text, "PlatformAccessibilityTreeUpdate") ||
      !contains(win32_text, "update_accessibility_tree(") ||
      !contains(win32_text, "focused_node_count") ||
      !contains(win32_text, "PlatformAccessibilityRole::text_input")) {
    return 56;
  }
  if (!contains(win32_text, "struct Win32UiaProviderNode") ||
      !contains(win32_text, "provider_nodes_") ||
      !contains(win32_text, "uia_provider_nodes()") ||
      !contains(win32_text, "std::optional<Rect> bounds") ||
      !contains(win32_text, "std::string value") ||
      !contains(win32_text, "role = node.role") ||
      !contains(win32_text, "focused = node.focused")) {
    return 66;
  }
  if (!contains(win32_text, "PlatformAccessibilityLiveUpdate") ||
      !contains(win32_text, "live_updates_") ||
      !contains(win32_text, "last_live_updates()") ||
      !contains(win32_text, "live_updates_ = last_update_.live_updates")) {
    return 67;
  }
  if (!contains(win32_text, "DragEntered{") ||
      !contains(win32_text, "DragUpdated{") ||
      !contains(win32_text, "DragDropped{") ||
      !contains(win32_text, "DragExited{")) {
    return 53;
  }
  if (!contains(win32_text, "class Win32OleDropTarget final : public IDropTarget") ||
      !contains(win32_text, "Win32OleDropTargetRegistrationState") ||
      !contains(win32_text, "IDataObject") ||
      !contains(win32_text, "RegisterDragDrop(") ||
      !contains(win32_text, "RevokeDragDrop(")) {
    return 58;
  }
  if (!contains(win32_text, "DROPEFFECT_COPY") ||
      !contains(win32_text, "DROPEFFECT_MOVE") ||
      !contains(win32_text, "drag_action_from_drop_effect(") ||
      !contains(win32_text, "drag_payload_from_ole_data_object(")) {
    return 59;
  }
  if (!contains(win32_text, "last_registration_result") ||
      !contains(win32_text, "last_revocation_result") ||
      !contains(win32_text, "register_drop_target(") ||
      !contains(win32_text, "revoke_drop_target()")) {
    return 60;
  }
  if (!contains(win32_text, "CGPUI.Win32.TestDragEnter") ||
      !contains(win32_text, "CGPUI.Win32.TestDragUpdate") ||
      !contains(win32_text, "CGPUI.Win32.TestDragDrop") ||
      !contains(win32_text, "CGPUI.Win32.TestDragExit")) {
    return 54;
  }
  if (!contains(win32_text, "discover_font_discovery()") ||
      !contains(win32_text, "const override") ||
      !contains(win32_text, "PlatformFontDiscoveryBackend::direct_write") ||
      !contains(
          win32_text,
          "PlatformFontDiscoveryStatus::deterministic_fallback") ||
      !contains(win32_text, "Segoe UI") ||
      !contains(win32_text, "win32://Segoe UI")) {
    return 57;
  }
  if (!contains(win32_text, "Win32NativeMenuState") ||
      !contains(win32_text, "install_native_menu(") ||
      !contains(win32_text, "PlatformMenuInstallationResult") ||
      !contains(win32_text, "native_menu_item_count(") ||
      !contains(win32_text, "native_menu_accelerator_count(") ||
      !contains(win32_text, "last_menu_installation_")) {
    return 61;
  }
  if (!contains(win32_text, "backend = \"win32\"") ||
      !contains(win32_text, "accelerator_count")) {
    return 62;
  }
  if (!contains(win32_text, "Win32NativeFileDialogState") ||
      !contains(win32_text, "show_native_file_dialog(") ||
      !contains(win32_text, "NativeFileDialogResult") ||
      !contains(win32_text, "last_file_dialog_result_")) {
    return 63;
  }
  if (!contains(win32_text, "backend = \"win32\"") ||
      !contains(win32_text, "filter_count")) {
    return 64;
  }
  if (!contains(win32_text, "Win32WindowChromeState") ||
      !contains(win32_text, "apply_window_chrome(") ||
      !contains(win32_text, "PlatformWindowChromeState") ||
      !contains(win32_text, "titlebar_visible") ||
      !contains(win32_text, "transparent_background")) {
    return 65;
  }

  const std::string win32_application =
      read_source("src/platform/win32/win32_application.cpp");
  const std::string win32_window_factory =
      read_source("src/platform/win32/win32_window_factory.cpp");
  const std::string win32_internal =
      read_source("src/platform/win32/win32_internal.hpp");
  const std::string win32_accessibility_internal =
      read_source("src/platform/win32/win32_accessibility_internal.hpp");
  const std::string win32_uia_provider_internal =
      read_source("src/platform/win32/win32_uia_provider_internal.hpp");
  const std::string win32_uia_provider_object_internal = read_source(
      "src/platform/win32/win32_uia_provider_object_internal.hpp");
  const std::string win32_uia_navigation_internal = read_source(
      "src/platform/win32/win32_uia_navigation_internal.hpp");
  const std::string win32_drag_drop_internal =
      read_source("src/platform/win32/win32_drag_drop_internal.hpp");
  const std::string win32_native_internal =
      read_source("src/platform/win32/win32_native_internal.hpp");
  const std::string win32_window_message_internal =
      read_source("src/platform/win32/win32_window_message_internal.hpp");
  const std::string win32_window_internal =
      read_source("src/platform/win32/win32_window_internal.hpp");
  const std::string win32_accessibility =
      read_source("src/platform/win32/win32_accessibility.cpp");
  const std::string win32_uia_provider =
      read_source("src/platform/win32/win32_uia_provider.cpp");
  const std::string win32_uia_fragment =
      read_source("src/platform/win32/win32_uia_fragment.cpp");
  const std::string win32_uia_navigation =
      read_source("src/platform/win32/win32_uia_navigation.cpp");
  const std::string win32_helpers =
      read_source("src/platform/win32/win32_helpers.cpp");
  const std::string win32_string =
      read_source("src/platform/win32/win32_string.cpp");
  const std::string win32_drag_drop_helpers =
      read_source("src/platform/win32/win32_drag_drop_helpers.cpp");
  const std::string win32_drag_drop_payload =
      read_source("src/platform/win32/win32_drag_drop_payload.cpp");
  const std::string win32_drag_drop_ole_payload =
      read_source("src/platform/win32/win32_drag_drop_ole_payload.cpp");
  const std::string win32_input_helpers =
      read_source("src/platform/win32/win32_input_helpers.cpp");
  const std::string win32_cursor =
      read_source("src/platform/win32/win32_cursor.cpp");
  const std::string win32_font_discovery =
      read_source("src/platform/win32/win32_font_discovery.cpp");
  const std::string win32_native =
      read_source("src/platform/win32/win32_native.cpp");
  const std::string win32_ole_drop_target =
      read_source("src/platform/win32/win32_ole_drop_target.cpp");
  const std::string win32_ole_drop_source =
      read_source("src/platform/win32/win32_ole_drop_source.cpp");
  const std::string win32_window =
      read_source("src/platform/win32/win32_window.cpp");
  const std::string win32_window_accessibility =
      read_source("src/platform/win32/win32_window_accessibility.cpp");
  const std::string win32_window_chrome =
      read_source("src/platform/win32/win32_window_chrome.cpp");
  const std::string win32_window_size =
      read_source("src/platform/win32/win32_window_size.cpp");
  const std::string win32_window_cursor =
      read_source("src/platform/win32/win32_window_cursor.cpp");
  const std::string win32_window_drag_drop =
      read_source("src/platform/win32/win32_window_drag_drop.cpp");
  const std::string win32_window_pointer_events =
      read_source("src/platform/win32/win32_window_pointer_events.cpp");
  const std::string win32_window_events =
      read_source("src/platform/win32/win32_window_events.cpp");
  const std::string win32_window_text =
      read_source("src/platform/win32/win32_window_text.cpp");
  const std::string win32_window_ime =
      read_source("src/platform/win32/win32_window_ime.cpp");
  const std::string win32_window_ime_placement =
      read_source("src/platform/win32/win32_window_ime_placement.cpp");
  const std::string win32_window_proc =
      read_source("src/platform/win32/win32_window_proc.cpp");
  const std::string win32_window_proc_accessibility_internal = read_source(
      "src/platform/win32/win32_window_proc_accessibility_internal.hpp");
  const std::string win32_window_proc_accessibility = read_source(
      "src/platform/win32/win32_window_proc_accessibility.cpp");
  const std::string win32_window_proc_drag =
      read_source("src/platform/win32/win32_window_proc_drag.cpp");
  const std::string win32_window_proc_lifecycle =
      read_source("src/platform/win32/win32_window_proc_lifecycle.cpp");
  const std::string win32_window_proc_pointer =
      read_source("src/platform/win32/win32_window_proc_pointer.cpp");
  const std::string win32_window_proc_cursor =
      read_source("src/platform/win32/win32_window_proc_cursor.cpp");
  const std::string win32_pointer_button =
      read_source("src/platform/win32/win32_pointer_button.cpp");
  const std::string win32_pointer_scroll =
      read_source("src/platform/win32/win32_pointer_scroll.cpp");
  const std::string win32_keyboard_key =
      read_source("src/platform/win32/win32_keyboard_key.cpp");
  const std::string win32_dead_key =
      read_source("src/platform/win32/win32_dead_key.cpp");
  const std::string win32_text_input =
      read_source("src/platform/win32/win32_text_input.cpp");
  const std::string win32_window_proc_keyboard =
      read_source("src/platform/win32/win32_window_proc_keyboard.cpp");
  const std::string win32_window_proc_text =
      read_source("src/platform/win32/win32_window_proc_text.cpp");
  if (win32_application.empty() || win32_internal.empty() ||
      win32_accessibility_internal.empty() ||
      win32_uia_provider_internal.empty() ||
      win32_uia_provider_object_internal.empty() ||
      win32_uia_navigation_internal.empty() || win32_uia_provider.empty() ||
      win32_uia_fragment.empty() || win32_uia_navigation.empty() ||
      win32_drag_drop_internal.empty() || win32_native_internal.empty() ||
      win32_window_message_internal.empty() ||
      win32_window_internal.empty() ||
      win32_accessibility.empty() || win32_helpers.empty() ||
      win32_string.empty() || win32_drag_drop_helpers.empty() ||
      win32_drag_drop_payload.empty() ||
      win32_drag_drop_ole_payload.empty() ||
      win32_input_helpers.empty() || win32_cursor.empty() ||
      win32_font_discovery.empty() ||
      win32_native.empty() ||
      win32_ole_drop_target.empty() || win32_ole_drop_source.empty() ||
      win32_window.empty() || win32_window_accessibility.empty() ||
      win32_window_chrome.empty() || win32_window_size.empty() ||
      win32_window_cursor.empty() ||
      win32_window_drag_drop.empty() || win32_window_pointer_events.empty() ||
      win32_window_events.empty() || win32_window_text.empty() ||
      win32_window_ime.empty() || win32_window_ime_placement.empty() ||
      win32_window_proc.empty() ||
      win32_window_proc_accessibility_internal.empty() ||
      win32_window_proc_accessibility.empty() ||
      win32_window_proc_drag.empty() ||
      win32_window_proc_lifecycle.empty() ||
      win32_window_proc_pointer.empty() || win32_window_proc_cursor.empty() ||
      win32_pointer_button.empty() || win32_pointer_scroll.empty() ||
      win32_keyboard_key.empty() || win32_dead_key.empty() ||
      win32_text_input.empty() || win32_window_proc_keyboard.empty() ||
      win32_window_proc_text.empty()) {
    return 68;
  }
  if (line_count(win32_internal) > 80 ||
      !contains(win32_internal, "#include \"win32_accessibility_internal.hpp\"") ||
      !contains(win32_internal, "#include \"win32_drag_drop_internal.hpp\"") ||
      !contains(win32_internal, "#include \"win32_native_internal.hpp\"") ||
      !contains(win32_internal,
                "#include \"win32_window_message_internal.hpp\"") ||
      contains(win32_internal, "class Win32OleDropTarget final") ||
      contains(win32_internal, "class Win32UiaAccessibilityAdapter") ||
      contains(win32_internal, "class Win32NativeMenuState")) {
    return 85;
  }
  if (line_count(win32_accessibility_internal) > 80 ||
      !contains(win32_accessibility_internal,
                "#include \"win32_uia_provider_internal.hpp\"") ||
      !contains(
          win32_accessibility_internal,
          "class Win32UiaAccessibilityAdapter") ||
      contains(win32_accessibility_internal, "IRawElementProviderSimple :") ||
      contains(win32_accessibility_internal, "class Win32OleDropTarget")) {
    return 86;
  }
  if (line_count(win32_uia_provider_internal) > 60 ||
      !contains(win32_uia_provider_internal, "struct Win32UiaProviderNode") ||
      !contains(win32_uia_provider_internal, "create_win32_uia_provider(") ||
      contains(win32_uia_provider_internal,
               "class Win32UiaAccessibilityAdapter")) return 96;
  if (line_count(win32_uia_provider_object_internal) > 90 ||
      !contains(win32_uia_provider_object_internal,
                "class Win32UiaProvider final") ||
      !contains(win32_uia_provider_object_internal,
                "public IRawElementProviderFragment") ||
      !contains(win32_uia_provider_object_internal,
                "public IRawElementProviderFragmentRoot") ||
      line_count(win32_uia_navigation_internal) > 70 ||
      !contains(win32_uia_navigation_internal,
                "navigate_win32_uia_fragment(")) return 98;
  if (line_count(win32_drag_drop_internal) > 120 ||
      !contains(win32_drag_drop_internal, "Win32TestDragDropPayload") ||
      !contains(win32_drag_drop_internal,
                "class Win32OleDropTarget final") ||
      !contains(win32_drag_drop_internal,
                "class Win32OleDropTargetOwner") ||
      contains(win32_drag_drop_internal, "class Win32NativeMenuState")) {
    return 87;
  }
  if (line_count(win32_native_internal) > 70 ||
      !contains(win32_native_internal, "class Win32NativeMenuState") ||
      !contains(win32_native_internal, "class Win32NativeFileDialogState") ||
      contains(win32_native_internal, "class Win32OleDropTarget")) {
    return 88;
  }
  if (line_count(win32_window_message_internal) > 90 ||
      !contains(win32_window_message_internal, "Win32WindowChromeState") ||
      !contains(win32_window_message_internal,
                "class Win32WindowMessageTarget") ||
      contains(win32_window_message_internal, "class Win32NativeMenuState")) {
    return 89;
  }
  if (line_count(win32_application) > 220) {
    return 69;
  }
  if (contains(win32_application, "class Win32Window final") ||
      contains(win32_application, "Win32Window::") ||
      contains(win32_application, "class Win32OleDropTarget final") ||
      contains(win32_application, "class Win32UiaAccessibilityAdapter") ||
      contains(win32_application, "class Win32NativeMenuState") ||
      contains(win32_application, "class Win32NativeFileDialogState") ||
      contains(win32_application, "LRESULT CALLBACK window_proc(")) {
    return 70;
  }
  if (line_count(win32_helpers) > 60 ||
      contains(win32_helpers, "std::wstring widen(") ||
      contains(win32_helpers, "KeyboardModifiers current_modifiers()") ||
      contains(win32_helpers,
               "DragDropPayload drag_payload_from_ole_data_object(")) {
    return 81;
  }
  if (!contains(win32_string, "std::wstring widen(") ||
      !contains(win32_string, "std::string utf8_from_utf16(") ||
      contains(win32_string, "drag_payload_from_ole_data_object(")) {
    return 82;
  }
  if (line_count(win32_drag_drop_helpers) > 90 ||
      !contains(win32_drag_drop_helpers, "test_drag_enter_message(") ||
      !contains(win32_drag_drop_helpers,
                "DragDropAction drag_action_from_test_hook(") ||
      !contains(win32_drag_drop_helpers, "choose_ole_drop_effect(") ||
      contains(win32_drag_drop_helpers,
               "DragDropPayload drag_payload_from_test_hook(") ||
      contains(win32_drag_drop_helpers,
               "DragDropPayload drag_payload_from_ole_data_object(") ||
      contains(win32_drag_drop_helpers, "KeyboardModifiers current_modifiers(")) {
    return 83;
  }
  if (line_count(win32_drag_drop_payload) > 50 ||
      !contains(win32_drag_drop_payload,
                "DragDropPayload drag_payload_from_test_hook(") ||
      !contains(win32_drag_drop_payload,
                "Point drag_position_from_test_hook(") ||
      contains(win32_drag_drop_payload,
               "DragDropPayload drag_payload_from_ole_data_object(") ||
      contains(win32_drag_drop_payload, "choose_ole_drop_effect(") ||
      contains(win32_drag_drop_payload, "RegisterWindowMessageW(")) {
    return 85;
  }
  if (line_count(win32_drag_drop_ole_payload) > 90 ||
      !contains(win32_drag_drop_ole_payload,
                "DragDropPayload drag_payload_from_ole_data_object(") ||
      !contains(win32_drag_drop_ole_payload, "CF_UNICODETEXT") ||
      !contains(win32_drag_drop_ole_payload, "CF_HDROP") ||
      contains(win32_drag_drop_ole_payload,
               "DragDropPayload drag_payload_from_test_hook(") ||
      contains(win32_drag_drop_ole_payload, "choose_ole_drop_effect(")) {
    return 86;
  }
  if (!contains(win32_input_helpers, "KeyboardModifiers current_modifiers()") ||
      contains(win32_input_helpers, "cursor_id_for(") ||
      !contains(win32_cursor, "win32_system_cursor_id(") ||
      !contains(win32_input_helpers, "DWORD win32_window_style_for(") ||
      contains(win32_input_helpers,
               "DragDropPayload drag_payload_from_ole_data_object(")) {
    return 84;
  }
  if (!contains(win32_font_discovery, "win32_discover_fonts()") ||
      !contains(
          win32_font_discovery,
          "PlatformFontDiscoveryBackend::direct_write") ||
      !contains(
          win32_font_discovery,
          "PlatformFontDiscoveryStatus::deterministic_fallback") ||
      !contains(win32_font_discovery, "win32://Segoe UI") ||
      contains(win32_application, "FontFaceDescriptor{")) {
    return 94;
  }
  if (!contains(win32_window_internal, "class Win32Window final") ||
      !contains(win32_window_internal, "public PlatformWindow") ||
      !contains(win32_window_internal, "public Win32OleDropTargetOwner") ||
      !contains(win32_window_internal, "public Win32WindowMessageTarget")) {
    return 75;
  }
  if (!contains(win32_window, "Win32Window::Win32Window(") ||
      !contains(win32_window, "Win32Window::~Win32Window()") ||
      !contains(win32_window, "Win32Window::native_surface() const") ||
      contains(win32_window, "Win32Window::apply_window_chrome(") ||
      contains(win32_window, "Win32Window::dpi_changed(") ||
      contains(win32_window, "Win32Window::update_size(")) {
    return 76;
  }
  if (!contains(win32_window_accessibility,
                "Win32Window::update_accessibility_tree(") ||
      !contains(win32_window_accessibility,
                "Win32Window::accessibility_object(") ||
      contains(win32_window, "Win32Window::update_accessibility_tree(")) {
    return 99;
  }
  if (!contains(win32_window_chrome,
                "Win32Window::apply_window_chrome(") ||
      !contains(win32_window_chrome, "win32_window_style_for(") ||
      !contains(win32_window_chrome, "SetWindowLongPtrW") ||
      !contains(win32_window_chrome, "transparent_background") ||
      contains(win32_window_chrome, "Win32Window::dpi_changed(") ||
      contains(win32_window_chrome, "Win32Window::pointer_moved(")) {
    return 90;
  }
  if (!contains(win32_window_size, "Win32Window::update_size()") ||
      !contains(win32_window_size, "Win32Window::update_size_for_dpi(") ||
      !contains(win32_window_size, "Win32Window::dpi_changed(") ||
      !contains(win32_window_size, "WindowResized{") ||
      contains(win32_window_size, "Win32Window::apply_window_chrome(") ||
      contains(win32_window_size, "Win32Window::pointer_moved(")) {
    return 91;
  }
  if (!contains(win32_window_pointer_events, "Win32Window::pointer_moved(") ||
      !contains(win32_window_pointer_events, "Win32Window::pointer_button(") ||
      !contains(win32_window_pointer_events, "Win32Window::pointer_scrolled(") ||
      contains(win32_window_events, "Win32Window::pointer_moved(") ||
      contains(win32_window_events, "Win32Window::pointer_button(") ||
      contains(win32_window_events, "Win32Window::pointer_scrolled(") ||
      !contains(win32_window_events, "Win32Window::key_event(") ||
      contains(win32_window_events, "Win32Window::text_input(") ||
      !contains(win32_window_events, "Win32Window::drag_dropped(")) {
    return 77;
  }
  if (!contains(win32_window_text, "Win32Window::dead_key(") ||
      !contains(win32_window_text, "Win32Window::text_input(")) {
    return 94;
  }
  if (!contains(win32_window_ime, "Win32Window::ime_start_composition()") ||
      !contains(win32_window_ime, "Win32Window::ime_composition(") ||
      !contains(win32_window_ime, "Win32Window::ime_end_composition()") ||
      !contains(win32_window_ime, "ImmGetCompositionStringW(") ||
      contains(win32_window_ime, "ImmSetCompositionWindow(")) {
    return 78;
  }
  if (!contains(win32_window_ime_placement,
                "Win32Window::apply_ime_text_input_placement()") ||
      !contains(win32_window_ime_placement, "ImmSetCompositionWindow(") ||
      !contains(win32_window_ime_placement, "ImmSetCandidateWindow(") ||
      contains(win32_window_ime_placement, "ImmGetCompositionStringW(")) {
    return 95;
  }
  if (!contains(win32_window_drag_drop, "Win32Window::ole_drag_entered(") ||
      !contains(win32_window_drag_drop, "Win32Window::ole_drag_dropped(") ||
      !contains(win32_window_drag_drop, "Win32Window::register_drop_target(") ||
      !contains(win32_window_drag_drop, "Win32Window::revoke_drop_target()") ||
      !contains(win32_window_drag_drop, "Win32Window::client_position_from_screen(")) {
    return 79;
  }
  if (line_count(win32_window) > 130 ||
      line_count(win32_window_chrome) > 90 ||
      line_count(win32_window_size) > 80 ||
      line_count(win32_window_pointer_events) > 55 ||
      line_count(win32_window_events) > 180 ||
      line_count(win32_window_text) > 45 ||
      line_count(win32_window_ime) > 90 ||
      line_count(win32_window_ime_placement) > 90 ||
      line_count(win32_window_drag_drop) > 170) {
    return 80;
  }
  if (!contains(win32_accessibility, "Win32UiaAccessibilityAdapter::update(") ||
      !contains(win32_accessibility, "create_win32_uia_provider(") ||
      !contains(win32_accessibility, "provider_for_element(") ||
      !contains(win32_accessibility, "text_input_node_count_") ||
      line_count(win32_accessibility) > 120) {
    return 71;
  }
  if (line_count(win32_uia_provider) > 190 ||
      !contains(win32_uia_provider, "Win32UiaProvider::QueryInterface(") ||
      !contains(win32_uia_provider, "UiaHostProviderFromHwnd(") ||
      !contains(win32_uia_provider, "UIA_AutomationIdPropertyId") ||
      !contains(win32_uia_provider, "UIA_ControlTypePropertyId") ||
      !contains(win32_uia_provider, "UIA_BoundingRectanglePropertyId")) return 97;
  if (line_count(win32_uia_fragment) > 100 ||
      !contains(win32_uia_fragment, "Win32UiaProvider::Navigate(") ||
      !contains(win32_uia_fragment, "Win32UiaProvider::GetRuntimeId(") ||
      !contains(win32_uia_fragment, "Win32UiaProvider::GetFocus(") ||
      line_count(win32_uia_navigation) > 230 ||
      !contains(win32_uia_navigation, "class Win32UiaProviderTree") ||
      !contains(win32_uia_navigation, "NavigateDirection_FirstChild") ||
      !contains(win32_uia_navigation, "get_win32_uia_provider_from_point(")) {
    return 100;
  }
  if (!contains(win32_native, "Win32NativeMenuState::install_native_menu(") ||
      !contains(win32_native, "Win32NativeFileDialogState::show_native_file_dialog(") ||
      !contains(win32_native, "backend = \"win32\"")) {
    return 72;
  }
  if (!contains(win32_ole_drop_target, "Win32OleDropTarget::DragEnter(") ||
      !contains(win32_ole_drop_target, "Win32OleDropTargetOwner") ||
      !contains(win32_ole_drop_target, "owner_->ole_drag_entered(")) {
    return 73;
  }
  if (!contains(win32_window_proc, "LRESULT CALLBACK win32_window_proc(") ||
      !contains(win32_window_proc, "Win32WindowMessageTarget") ||
      !contains(win32_window_proc, "win32_window_proc_handle_test_drag(") ||
      !contains(win32_window_proc,
                "win32_window_proc_handle_accessibility(") ||
      !contains(win32_window_proc, "win32_window_proc_handle_lifecycle(") ||
      !contains(win32_window_proc, "win32_window_proc_handle_pointer(") ||
      !contains(win32_window_proc, "win32_window_proc_handle_keyboard(") ||
      !contains(win32_window_factory, "win32_window_proc") ||
      !contains(win32_window_factory, "CreateWindowExW(")) {
    return 74;
  }
  if (line_count(win32_window_proc) > 80 ||
      contains(win32_window_proc, "test_drag_enter_message()") ||
      contains(win32_window_proc, "WM_NCCREATE") ||
      contains(win32_window_proc, "WM_MOUSEWHEEL") ||
      contains(win32_window_proc, "WM_KEYDOWN") ||
      contains(win32_window_proc, "WM_CHAR")) {
    return 92;
  }
  if (line_count(win32_window_proc_accessibility_internal) > 45 ||
      !contains(win32_window_proc_accessibility_internal,
                "class Win32AccessibilityMessageTarget") ||
      line_count(win32_window_proc_accessibility) > 35 ||
      !contains(win32_window_proc_accessibility, "WM_GETOBJECT") ||
      !contains(win32_window_proc_accessibility, "accessibility_object(")) {
    return 101;
  }
  if (!contains(win32_window_proc_drag, "test_drag_enter_message()") ||
      !contains(win32_window_proc_drag, "drag_entered(") ||
      !contains(win32_window_proc_lifecycle, "WM_NCCREATE") ||
      !contains(win32_window_proc_lifecycle, "WM_PAINT") ||
      !contains(win32_window_proc_lifecycle, "WM_NCDESTROY") ||
      !contains(win32_window_proc_pointer, "decode_win32_pointer_scroll(") ||
      !contains(win32_pointer_button, "WM_LBUTTONDOWN") ||
      !contains(win32_pointer_button, "WM_XBUTTONDOWN") ||
      !contains(win32_pointer_button, "WM_LBUTTONDBLCLK") ||
      !contains(win32_pointer_scroll, "WM_MOUSEWHEEL") ||
      !contains(win32_pointer_scroll, "WM_MOUSEHWHEEL") ||
      !contains(win32_window_cursor, "Win32Window::refresh_cursor(") ||
      !contains(win32_window_proc_cursor, "WM_SETCURSOR") ||
      !contains(win32_window_proc_keyboard, "decode_win32_keyboard_key(") ||
      !contains(win32_window_proc_keyboard, "decode_win32_dead_key(") ||
      !contains(win32_keyboard_key, "WM_KEYDOWN") ||
      !contains(win32_keyboard_key, "WM_SYSKEYDOWN") ||
      !contains(win32_dead_key, "WM_DEADCHAR") ||
      !contains(win32_dead_key, "WM_SYSDEADCHAR") ||
      contains(win32_window_proc_keyboard, "WM_CHAR") ||
      !contains(win32_text_input, "WM_UNICHAR") ||
      !contains(win32_window_proc_text, "decode_win32_text_input(")) {
    return 93;
  }

  const std::string xmake_text = read_source("xmake.lua");
  if (xmake_text.empty()) {
    return 9;
  }

  const std::string platform_target =
      target_block(xmake_text, "cgpui_platform_win32");
  if (platform_target.empty()) {
    return 10;
  }
  if (!contains(platform_target, "src/platform/win32/*.cpp")) {
    return 11;
  }
  if (!contains(platform_target, "cgpui_core") ||
      !contains(platform_target, "cgpui_platform")) {
    return 12;
  }
  if (!contains(platform_target, "\"user32\"") ||
      !contains(platform_target, "\"gdi32\"") ||
      !contains(platform_target, "\"shell32\"") ||
      !contains(platform_target, "\"ole32\"")) {
    return 13;
  }

  const std::string input_target =
      target_block(xmake_text, "win32_input_event_test");
  if (input_target.empty()) {
    return 14;
  }
  if (!contains(input_target, "tests/platform/win32_input_event_test.cpp")) {
    return 15;
  }
  if (!contains(input_target, "cgpui_platform_win32")) {
    return 16;
  }
  if (!contains(input_target, "add_tests(\"default\")")) {
    return 17;
  }

  const std::string dpi_target =
      target_block(xmake_text, "win32_dpi_scale_test");
  if (dpi_target.empty()) {
    return 33;
  }
  if (!contains(dpi_target, "tests/platform/win32_dpi_scale_test.cpp")) {
    return 34;
  }
  if (!contains(dpi_target, "cgpui_platform_win32")) {
    return 35;
  }
  if (!contains(dpi_target, "add_tests(\"default\")")) {
    return 36;
  }

  const std::string focus_target =
      target_block(xmake_text, "win32_focus_event_test");
  if (focus_target.empty()) {
    return 40;
  }
  if (!contains(focus_target, "tests/platform/win32_focus_event_test.cpp")) {
    return 41;
  }
  if (!contains(focus_target, "cgpui_platform_win32")) {
    return 42;
  }
  if (!contains(focus_target, "add_tests(\"default\")")) {
    return 43;
  }

  const std::string text_input_target =
      target_block(xmake_text, "win32_text_input_test");
  if (text_input_target.empty()) {
    return 47;
  }
  if (!contains(text_input_target, "tests/platform/win32_text_input_test.cpp")) {
    return 48;
  }
  if (!contains(text_input_target, "cgpui_platform_win32")) {
    return 49;
  }
  if (line_count(win32_ole_drop_source) > 90 ||
      !contains(win32_ole_drop_source,
                "Win32OleDropSource::QueryContinueDrag(") ||
      !contains(win32_ole_drop_source, "run_win32_ole_drag(")) return 90;
  if (!contains(text_input_target, "add_tests(\"default\")")) {
    return 50;
  }

  const std::string surface_target =
      target_block(xmake_text, "win32_vulkan_surface_test");
  if (surface_target.empty()) {
    return 18;
  }
  if (!contains(surface_target, "tests/renderer/win32_vulkan_surface_test.cpp")) {
    return 19;
  }
  if (!contains(surface_target, "cgpui_platform_win32")) {
    return 20;
  }
  if (!contains(surface_target, "cgpui_renderer_vulkan")) {
    return 21;
  }
  if (!contains(surface_target, "add_tests(\"default\")")) {
    return 22;
  }

  const std::string hello_target =
      target_block(xmake_text, "hello_window");
  if (hello_target.empty()) {
    return 23;
  }

  const std::string windows_branch =
      platform_branch(hello_target, "windows");
  if (windows_branch.empty()) {
    return 24;
  }
  if (!contains(windows_branch, "cgpui_platform_win32")) {
    return 25;
  }
  if (!contains(windows_branch, "cgpui_renderer_vulkan")) {
    return 26;
  }
  if (contains(windows_branch, "cgpui_platform_fallback") ||
      contains(windows_branch, "cgpui_renderer_fallback")) {
    return 27;
  }
  if (!contains(windows_branch, "CGPUI_EXIT_AFTER_FIRST_FRAME") ||
      !contains(windows_branch, "add_tests(\"windows_first_frame\"")) {
    return 28;
  }
  if (!contains(windows_branch, "CGPUI_RESIZE_AFTER_FIRST_FRAME") ||
      !contains(windows_branch,
                "add_tests(\"windows_resize_after_first_frame\"")) {
    return 29;
  }
  if (!contains(windows_branch, "CGPUI_CLOSE_AFTER_FIRST_FRAME") ||
      !contains(windows_branch,
                "add_tests(\"windows_close_after_first_frame\"")) {
    return 30;
  }

  return 0;
}
