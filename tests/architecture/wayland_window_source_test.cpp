#include <fstream>
#include <iterator>
#include <algorithm>
#include <string>

namespace {

std::string read_wayland_source() {
  std::ifstream source("src/platform/linux/wayland_application.cpp");
  if (!source) {
    source.open("../../../../src/platform/linux/wayland_application.cpp");
  }
  if (!source) {
    return {};
  }

  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

std::string read_xmake_source() {
  std::ifstream source("xmake.lua");
  if (!source) {
    source.open("../../../../xmake.lua");
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

std::string linux_target_block(const std::string& xmake_text, const char* target_name) {
  const std::string target_marker =
      std::string("target(\"") + target_name + "\")";
  const auto target_position = xmake_text.find(target_marker);
  if (target_position == std::string::npos) {
    return {};
  }

  const auto next_target_position = xmake_text.find("\ntarget(\"", target_position + 1);
  const auto next_platform_position =
      xmake_text.find("\nif is_plat(\"macosx\")", target_position + 1);
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
  const std::string text = read_wayland_source();
  if (text.empty()) {
    return 2;
  }

  if (contains(text, "Wayland surface creation is not implemented yet")) {
    return 1;
  }

  if (!contains(text, "WaylandWindow") ||
      !contains(text, "xdg_wm_base") ||
      !contains(text, "xdg_surface") ||
      !contains(text, "xdg_toplevel")) {
    return 3;
  }

  if (!contains(text, "WaylandSurfaceHandle{.display = display_")) {
    return 4;
  }
  if (!contains(text, ".scale = window->state_.scale")) {
    return 54;
  }
  if (!contains(text, "WindowFocused{.focused = focused}")) {
    return 55;
  }
  if (!contains(text, "handle_keyboard_enter") ||
      !contains(text, "focus_changed(true)")) {
    return 56;
  }
  if (!contains(text, "handle_keyboard_leave") ||
      !contains(text, "focus_changed(false)")) {
    return 57;
  }
  if (!contains(text, "#include <xkbcommon/xkbcommon.h>")) {
    return 58;
  }
  if (!contains(text, "xkb_state_update_mask")) {
    return 59;
  }
  if (!contains(text, "xkb_state_key_get_utf8")) {
    return 60;
  }
  if (!contains(text, "TextInput{")) {
    return 61;
  }
  if (!contains(text, "discover_font_records()") ||
      !contains(text, "const override") ||
      !contains(text, "FontFaceDescriptor") ||
      !contains(text, "FontSource::platform") ||
      !contains(text, "sans-serif") ||
      !contains(text, "fontconfig://sans-serif")) {
    return 67;
  }
  if (!contains(text, "WaylandTextInputState") ||
      !contains(text, "WaylandTextInput") ||
      !contains(text, "ImeTextInputSupport::available") ||
      !contains(text, "set_ime_text_input_placement") ||
      !contains(text, "surrounding_text") ||
      !contains(text, "content_type") ||
      !contains(text, "preedit") ||
      !contains(text, "commit") ||
      !contains(text, "ImeComposition{")) {
    return 63;
  }
  if (!contains(text, "WaylandAtspiAccessibilityAdapter") ||
      !contains(text, "PlatformAccessibilityTreeUpdate") ||
      !contains(text, "update_accessibility_tree(") ||
      !contains(text, "focused_node_count") ||
      !contains(text, "PlatformAccessibilityRole::text_input") ||
      !contains(text, "window_->update_accessibility_tree(std::move(update))")) {
    return 66;
  }
  if (!contains(text, "struct WaylandAtspiObjectNode") ||
      !contains(text, "atspi_object_nodes_") ||
      !contains(text, "atspi_object_nodes()") ||
      !contains(text, "std::string object_path") ||
      !contains(text, "atspi_object_path_for(") ||
      !contains(text, "\"/org/a11y/atspi/accessible/\"") ||
      !contains(text, "role = node.role") ||
      !contains(text, "name = node.name") ||
      !contains(text, "text = node.text") ||
      !contains(text, "focused = node.focused")) {
    return 75;
  }
  if (!contains(text, "PlatformAccessibilityLiveUpdate") ||
      !contains(text, "live_updates_") ||
      !contains(text, "last_live_updates()") ||
      !contains(text, "live_updates_ = last_update_.live_updates")) {
    return 76;
  }
  if (!contains(text, "WaylandDataDevice") ||
      !contains(text, "wl_data_device_manager_get_data_device") ||
      !contains(text, "DragEntered{")) {
    return 64;
  }
  if (!contains(text, "WaylandCursorThemeState") ||
      !contains(text, "WaylandCursorThemeLoadStatus") ||
      !contains(text, "WaylandCursorImageState") ||
      !contains(text, "cursor_name_for_shape") ||
      !contains(text, "\"left_ptr\"") ||
      !contains(text, "\"hand2\"") ||
      !contains(text, "\"xterm\"") ||
      !contains(text, "cursor_theme_state_") ||
      !contains(text, "cursor_theme_status_") ||
      !contains(text, "cursor_image_unavailable")) {
    return 68;
  }
  if (!contains(text, "WaylandXdgConfigureState") ||
      !contains(text, "WaylandXdgToplevelState") ||
      !contains(text, "pending_configure_") ||
      !contains(text, "last_acked_configure_serial") ||
      !contains(text, "xdg_toplevel_state_activated") ||
      !contains(text, "xdg_toplevel_state_maximized") ||
      !contains(text, "xdg_toplevel_state_fullscreen") ||
      !contains(text, "record_toplevel_configure_state") ||
      !contains(text, "dispatch_configure_lifecycle_events")) {
    return 69;
  }
  if (!contains(text, "wakeup_pipe_") ||
      !contains(text, "poll(fds.data()") ||
      !contains(text, "WindowWakeupRequested{}")) {
    return 65;
  }
  if (!contains(text, "WaylandNativeMenuState") ||
      !contains(text, "install_native_menu(") ||
      !contains(text, "PlatformMenuInstallationResult") ||
      !contains(text, "native_menu_item_count(") ||
      !contains(text, "native_menu_accelerator_count(") ||
      !contains(text, "last_menu_installation_")) {
    return 70;
  }
  if (!contains(text, "backend = \"wayland\"") ||
      !contains(text, "accelerator_count")) {
    return 71;
  }
  if (!contains(text, "WaylandNativeFileDialogState") ||
      !contains(text, "show_native_file_dialog(") ||
      !contains(text, "NativeFileDialogResult") ||
      !contains(text, "last_file_dialog_result_")) {
    return 72;
  }
  if (!contains(text, "backend = \"wayland\"") ||
      !contains(text, "filter_count")) {
    return 73;
  }
  if (!contains(text, "WaylandWindowChromeState") ||
      !contains(text, "apply_window_chrome(") ||
      !contains(text, "PlatformWindowChromeState") ||
      !contains(text, "titlebar_visible") ||
      !contains(text, "transparent_background")) {
    return 74;
  }

  const std::string xmake_text = read_xmake_source();
  if (xmake_text.empty()) {
    return 5;
  }

  const std::string close_target_text =
      linux_target_block(xmake_text, "wayland_compositor_close_test");
  if (close_target_text.empty()) {
    return 6;
  }
  if (!contains(close_target_text, "tests/platform/wayland_compositor_close_test.cpp")) {
    return 7;
  }
  if (!contains(close_target_text, "wayland-server")) {
    return 8;
  }
  if (!contains(close_target_text, "add_tests(\"default\")")) {
    return 9;
  }
  if (!contains(close_target_text, "tests/platform/wayland_test_compositor.cpp")) {
    return 10;
  }

  const std::string resize_target_text =
      linux_target_block(xmake_text, "wayland_compositor_resize_test");
  if (resize_target_text.empty()) {
    return 11;
  }
  if (!contains(resize_target_text, "tests/platform/wayland_compositor_resize_test.cpp")) {
    return 12;
  }
  if (!contains(resize_target_text, "tests/platform/wayland_test_compositor.cpp")) {
    return 13;
  }
  if (!contains(resize_target_text, "wayland-server")) {
    return 14;
  }
  if (!contains(resize_target_text, "add_tests(\"default\")")) {
    return 15;
  }

  const std::string pointer_target_text =
      linux_target_block(xmake_text, "wayland_pointer_button_test");
  if (pointer_target_text.empty()) {
    return 37;
  }
  if (!contains(pointer_target_text, "tests/platform/wayland_pointer_button_test.cpp")) {
    return 38;
  }
  if (!contains(pointer_target_text, "tests/platform/wayland_test_compositor.cpp")) {
    return 39;
  }
  if (!contains(pointer_target_text, "wayland-server")) {
    return 40;
  }
  if (!contains(pointer_target_text, "add_tests(\"default\")")) {
    return 41;
  }

  const std::string scroll_target_text =
      linux_target_block(xmake_text, "wayland_pointer_scroll_test");
  if (scroll_target_text.empty()) {
    return 47;
  }
  if (!contains(scroll_target_text, "tests/platform/wayland_pointer_scroll_test.cpp")) {
    return 48;
  }
  if (!contains(scroll_target_text, "tests/platform/wayland_test_compositor.cpp")) {
    return 49;
  }
  if (!contains(scroll_target_text, "wayland-server")) {
    return 50;
  }
  if (!contains(scroll_target_text, "add_tests(\"default\")")) {
    return 51;
  }

  const std::string keyboard_target_text =
      linux_target_block(xmake_text, "wayland_keyboard_test");
  if (keyboard_target_text.empty()) {
    return 42;
  }
  if (!contains(keyboard_target_text, "tests/platform/wayland_keyboard_test.cpp")) {
    return 43;
  }
  if (!contains(keyboard_target_text, "tests/platform/wayland_test_compositor.cpp")) {
    return 44;
  }
  if (!contains(keyboard_target_text, "wayland-server")) {
    return 45;
  }
  if (!contains(keyboard_target_text, "add_tests(\"default\")")) {
    return 46;
  }
  if (!contains(xmake_text, "libxkbcommon")) {
    return 62;
  }

  const std::string vulkan_target_text =
      linux_target_block(xmake_text, "wayland_vulkan_surface_test");
  if (vulkan_target_text.empty()) {
    return 16;
  }
  if (!contains(vulkan_target_text, "tests/renderer/wayland_vulkan_surface_test.cpp")) {
    return 17;
  }
  if (!contains(vulkan_target_text, "cgpui_platform_linux_wayland")) {
    return 18;
  }
  if (!contains(vulkan_target_text, "cgpui_renderer_vulkan")) {
    return 19;
  }
  if (!contains(vulkan_target_text, "\"vulkan\"")) {
    return 20;
  }
  if (!contains(vulkan_target_text, "add_tests(\"default\")")) {
    return 21;
  }

  const std::string hello_window_target_text =
      linux_target_block(xmake_text, "hello_window");
  if (hello_window_target_text.empty()) {
    return 22;
  }
  if (!contains(hello_window_target_text, "cgpui_platform_linux_wayland")) {
    return 23;
  }
  if (!contains(hello_window_target_text, "cgpui_renderer_vulkan")) {
    return 24;
  }
  if (!contains(hello_window_target_text, "CGPUI_EXIT_AFTER_FIRST_FRAME")) {
    return 25;
  }
  if (!contains(hello_window_target_text, "add_tests(\"linux_first_frame\"")) {
    return 26;
  }

  const std::string linux_hello_window_branch =
      platform_branch(hello_window_target_text, "linux");
  if (linux_hello_window_branch.empty()) {
    return 27;
  }
  if (!contains(linux_hello_window_branch, "CGPUI_RESIZE_AFTER_FIRST_FRAME")) {
    return 28;
  }
  if (!contains(linux_hello_window_branch,
                "add_tests(\"linux_resize_after_first_frame\"")) {
    return 29;
  }
  if (!contains(linux_hello_window_branch, "CGPUI_CLOSE_AFTER_FIRST_FRAME")) {
    return 52;
  }
  if (!contains(linux_hello_window_branch,
                "add_tests(\"linux_close_after_first_frame\"")) {
    return 53;
  }

  const std::string windows_hello_window_branch =
      platform_branch(hello_window_target_text, "windows");
  if (windows_hello_window_branch.empty()) {
    return 30;
  }
  if (!contains(windows_hello_window_branch, "cgpui_platform_win32")) {
    return 31;
  }
  if (!contains(windows_hello_window_branch, "cgpui_renderer_vulkan")) {
    return 32;
  }
  if (!contains(windows_hello_window_branch, "CGPUI_EXIT_AFTER_FIRST_FRAME")) {
    return 33;
  }
  if (!contains(windows_hello_window_branch,
                "add_tests(\"windows_first_frame\"")) {
    return 34;
  }
  if (!contains(windows_hello_window_branch, "CGPUI_RESIZE_AFTER_FIRST_FRAME")) {
    return 35;
  }
  if (!contains(windows_hello_window_branch,
                "add_tests(\"windows_resize_after_first_frame\"")) {
    return 36;
  }

  return 0;
}
