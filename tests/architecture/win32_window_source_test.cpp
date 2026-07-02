#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>

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
  const std::string win32_text =
      read_source("src/platform/win32/win32_application.cpp");
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
  if (!contains(win32_text, "discover_font_records()") ||
      !contains(win32_text, "const override") ||
      !contains(win32_text, "FontFaceDescriptor") ||
      !contains(win32_text, "FontSource::platform") ||
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
