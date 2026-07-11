#include <cstdlib>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace {

std::filesystem::path executable_directory;

std::string read_file(const std::filesystem::path& path) {
  std::ifstream source(path);
  if (!source) {
    return {};
  }

  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

std::string read_source_from_project_root(
    std::filesystem::path directory,
    const std::filesystem::path& relative_path) {
  if (directory.empty()) {
    return {};
  }
  directory = std::filesystem::absolute(directory);
  for (; !directory.empty();
       directory = directory.parent_path()) {
    if (std::filesystem::exists(directory / ".git")) {
      const std::string text = read_file(directory / relative_path);
      if (!text.empty()) {
        return text;
      }
    }
    if (directory == directory.root_path()) {
      break;
    }
  }
  return {};
}

std::string read_source_from_nearest_match(
    std::filesystem::path directory,
    const std::filesystem::path& relative_path) {
  if (directory.empty()) {
    return {};
  }
  directory = std::filesystem::absolute(directory);
  for (; !directory.empty();
       directory = directory.parent_path()) {
    const std::string text = read_file(directory / relative_path);
    if (!text.empty()) {
      return text;
    }
    if (directory == directory.root_path()) {
      break;
    }
  }

  return {};
}

std::string read_source(const char* path) {
  const std::filesystem::path relative_path(path);
  if (const char* source_root = std::getenv("CGPUI_SOURCE_ROOT");
      source_root != nullptr) {
    const std::string text = read_file(
        std::filesystem::path(source_root) / relative_path);
    if (!text.empty()) {
      return text;
    }
  }

  std::string text =
      read_source_from_project_root(executable_directory, relative_path);
  if (!text.empty()) {
    return text;
  }

  text = read_source_from_project_root(
      std::filesystem::current_path(),
      relative_path);
  if (!text.empty()) {
    return text;
  }

  return read_source_from_nearest_match(
      std::filesystem::current_path(),
      relative_path);
}

std::string read_wayland_source() {
  const std::vector<const char*> wayland_files{
      "src/platform/linux/wayland_internal.hpp",
      "src/platform/linux/wayland_input_internal.hpp",
      "src/platform/linux/wayland_uri_list_internal.hpp",
      "src/platform/linux/wayland_cursor_theme_internal.hpp",
      "src/platform/linux/wayland_cursor_theme.cpp",
      "src/platform/linux/wayland_protocol_internal.hpp",
      "src/platform/linux/wayland_services_internal.hpp",
      "src/platform/linux/wayland_window_api.hpp",
      "src/platform/linux/wayland_protocol.cpp",
      "src/platform/linux/wayland_protocol_fractional_scale_internal.hpp",
      "src/platform/linux/wayland_protocol_fractional_scale_interfaces.cpp",
      "src/platform/linux/wayland_protocol_fractional_scale.cpp",
      "src/platform/linux/wayland_protocol_viewporter_interfaces.cpp",
      "src/platform/linux/wayland_protocol_viewporter.cpp",
      "src/platform/linux/wayland_protocol_xdg_interfaces.cpp",
      "src/platform/linux/wayland_protocol_xdg_surface.cpp",
      "src/platform/linux/wayland_protocol_xdg_toplevel.cpp",
      "src/platform/linux/wayland_protocol_xdg_toplevel_display.cpp",
      "src/platform/linux/wayland_protocol_xdg_toplevel_parent.cpp",
      "src/platform/linux/wayland_protocol_xdg_toplevel_size.cpp",
      "src/platform/linux/wayland_protocol_xdg_decoration.cpp",
      "src/platform/linux/wayland_protocol_xdg_decoration_interfaces.cpp",
      "src/platform/linux/wayland_protocol_xdg_wm_base.cpp",
      "src/platform/linux/wayland_protocol_text_input_interfaces.cpp",
      "src/platform/linux/wayland_protocol_text_input_manager.cpp",
      "src/platform/linux/wayland_protocol_text_input_requests.cpp",
      "src/platform/linux/wayland_protocol_text_input.cpp",
      "src/platform/linux/wayland_protocol_xdg.cpp",
      "src/platform/linux/wayland_window_internal.hpp",
      "src/platform/linux/wayland_registered_window_internal.hpp",
      "src/platform/linux/wayland_window_configure_internal.hpp",
      "src/platform/linux/wayland_window_creation_internal.hpp",
      "src/platform/linux/wayland_window_decoration_internal.hpp",
      "src/platform/linux/wayland_window_parent_internal.hpp",
      "src/platform/linux/wayland_window_scale_internal.hpp",
      "src/platform/linux/wayland_window_fractional_scale_internal.hpp",
      "src/platform/linux/wayland_window.cpp",
      "src/platform/linux/wayland_window_chrome.cpp",
      "src/platform/linux/wayland_window_parent.cpp",
      "src/platform/linux/wayland_window_scale.cpp",
      "src/platform/linux/wayland_window_fractional_scale.cpp",
      "src/platform/linux/wayland_window_configure.cpp",
      "src/platform/linux/wayland_window_events.cpp",
      "src/platform/linux/wayland_window_input_events.cpp",
      "src/platform/linux/wayland_window_drag_events.cpp",
      "src/platform/linux/wayland_window_text_events.cpp",
      "src/platform/linux/wayland_window_registered.cpp",
      "src/platform/linux/wayland_window_bridge.cpp",
      "src/platform/linux/wayland_window_state.hpp",
      "src/platform/linux/wayland_window_state.cpp",
      "src/platform/linux/wayland_text_input_internal.hpp",
      "src/platform/linux/wayland_text_input.cpp",
      "src/platform/linux/wayland_text_input_core.cpp",
      "src/platform/linux/wayland_text_input_events.cpp",
      "src/platform/linux/wayland_text_input_requests.cpp",
      "src/platform/linux/wayland_data_device_internal.hpp",
      "src/platform/linux/wayland_data_device.cpp",
      "src/platform/linux/wayland_data_device_offer.cpp",
      "src/platform/linux/wayland_data_device_drag_actions.cpp",
      "src/platform/linux/wayland_uri_list.cpp",
      "src/platform/linux/wayland_data_device_drag_events.cpp",
      "src/platform/linux/wayland_data_device_drag.cpp",
      "src/platform/linux/wayland_data_device_payload.cpp",
      "src/platform/linux/wayland_accessibility.cpp",
      "src/platform/linux/wayland_native.cpp",
      "src/platform/linux/wayland_event_loop.cpp",
      "src/platform/linux/wayland_registry.cpp",
      "src/platform/linux/wayland_keyboard.cpp",
      "src/platform/linux/wayland_application_internal.hpp",
      "src/platform/linux/wayland_application_core_internal.hpp",
      "src/platform/linux/wayland_application_registry_internal.hpp",
      "src/platform/linux/wayland_application_input_internal.hpp",
      "src/platform/linux/wayland_application_cursor_internal.hpp",
      "src/platform/linux/wayland_application_input.cpp",
      "src/platform/linux/wayland_application_seat.cpp",
      "src/platform/linux/wayland_application_keyboard.cpp",
      "src/platform/linux/wayland_application_pointer.cpp",
      "src/platform/linux/wayland_application_pointer_scroll.cpp",
      "src/platform/linux/wayland_pointer_scroll_frame_internal.hpp",
      "src/platform/linux/wayland_pointer_scroll_frame.cpp",
      "src/platform/linux/wayland_application_cursor.cpp",
      "src/platform/linux/wayland_application_windows.cpp",
      "src/platform/linux/wayland_application_window_registry.cpp",
      "src/platform/linux/wayland_application_window_creation_internal.hpp",
      "src/platform/linux/wayland_application_window_creation.cpp",
      "src/platform/linux/wayland_application_child_window.cpp",
      "src/platform/linux/wayland_application_lifecycle.cpp",
      "src/platform/linux/wayland_application_services.cpp",
      "src/platform/linux/wayland_font_discovery.cpp",
      "src/platform/linux/wayland_application_factory.cpp",
      "src/platform/linux/wayland_application.cpp",
  };

  std::string text;
  for (const char* path : wayland_files) {
    const std::string source = read_source(path);
    if (source.empty()) {
      return {};
    }
    text += source;
    text += '\n';
  }
  return text;
}

std::string read_xmake_source() {
  return read_source("xmake.lua");
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  return static_cast<std::size_t>(
      std::count(text.begin(), text.end(), '\n'));
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

int main(int argc, char** argv) {
  if (argc > 0 && argv[0] != nullptr) {
    executable_directory = std::filesystem::absolute(argv[0]).parent_path();
  }

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
      !contains(text, "wayland_window_focus_changed(*app->keyboard_window_, true)")) {
    return 56;
  }
  if (!contains(text, "handle_keyboard_leave") ||
      !contains(text, "wayland_window_focus_changed(*app->keyboard_window_, false)")) {
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
  if (!contains(text, "discover_font_discovery()") ||
      !contains(text, "const override") ||
      !contains(text, "PlatformFontDiscoveryBackend::fontconfig") ||
      !contains(
          text,
          "PlatformFontDiscoveryStatus::deterministic_fallback") ||
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
      !contains(text, "zxdg_decoration_manager_v1_interface") ||
      !contains(text, "zxdg_toplevel_decoration_v1_set_mode") ||
      !contains(text, "transparency_supported = true")) {
    return 74;
  }

  const std::string wayland_window =
      read_source("src/platform/linux/wayland_window.cpp");
  const std::string wayland_internal =
      read_source("src/platform/linux/wayland_internal.hpp");
  const std::string wayland_input_internal =
      read_source("src/platform/linux/wayland_input_internal.hpp");
  const std::string wayland_protocol_internal =
      read_source("src/platform/linux/wayland_protocol_internal.hpp");
  const std::string wayland_protocol_xdg_interfaces =
      read_source("src/platform/linux/wayland_protocol_xdg_interfaces.cpp");
  const std::string wayland_protocol_xdg_surface =
      read_source("src/platform/linux/wayland_protocol_xdg_surface.cpp");
  const std::string wayland_protocol_xdg_toplevel =
      read_source("src/platform/linux/wayland_protocol_xdg_toplevel.cpp");
  const std::string wayland_protocol_xdg_wm_base =
      read_source("src/platform/linux/wayland_protocol_xdg_wm_base.cpp");
  const std::string wayland_protocol_xdg =
      read_source("src/platform/linux/wayland_protocol_xdg.cpp");
  const std::string wayland_protocol_text_input_interfaces =
      read_source("src/platform/linux/wayland_protocol_text_input_interfaces.cpp");
  const std::string wayland_protocol_text_input_manager =
      read_source("src/platform/linux/wayland_protocol_text_input_manager.cpp");
  const std::string wayland_protocol_text_input_requests =
      read_source("src/platform/linux/wayland_protocol_text_input_requests.cpp");
  const std::string wayland_protocol_text_input =
      read_source("src/platform/linux/wayland_protocol_text_input.cpp");
  const std::string wayland_services_internal =
      read_source("src/platform/linux/wayland_services_internal.hpp");
  const std::string wayland_window_api =
      read_source("src/platform/linux/wayland_window_api.hpp");
  const std::string wayland_window_internal =
      read_source("src/platform/linux/wayland_window_internal.hpp");
  const std::string wayland_registered_window_internal =
      read_source("src/platform/linux/wayland_registered_window_internal.hpp");
  const std::string wayland_window_configure_internal =
      read_source("src/platform/linux/wayland_window_configure_internal.hpp");
  const std::string wayland_window_configure =
      read_source("src/platform/linux/wayland_window_configure.cpp");
  const std::string wayland_window_events =
      read_source("src/platform/linux/wayland_window_events.cpp");
  const std::string wayland_window_input_events =
      read_source("src/platform/linux/wayland_window_input_events.cpp");
  const std::string wayland_window_drag_events =
      read_source("src/platform/linux/wayland_window_drag_events.cpp");
  const std::string wayland_window_text_events =
      read_source("src/platform/linux/wayland_window_text_events.cpp");
  const std::string wayland_window_registered =
      read_source("src/platform/linux/wayland_window_registered.cpp");
  const std::string wayland_window_bridge =
      read_source("src/platform/linux/wayland_window_bridge.cpp");
  const std::string wayland_window_state_header =
      read_source("src/platform/linux/wayland_window_state.hpp");
  const std::string wayland_window_state =
      read_source("src/platform/linux/wayland_window_state.cpp");
  if (wayland_window.empty() || wayland_internal.empty() ||
      wayland_input_internal.empty() || wayland_protocol_internal.empty() ||
      wayland_protocol_xdg_interfaces.empty() ||
      wayland_protocol_xdg_surface.empty() ||
      wayland_protocol_xdg_toplevel.empty() ||
      wayland_protocol_xdg_wm_base.empty() ||
      wayland_protocol_xdg.empty() ||
      wayland_protocol_text_input_interfaces.empty() ||
      wayland_protocol_text_input_manager.empty() ||
      wayland_protocol_text_input_requests.empty() ||
      wayland_protocol_text_input.empty() ||
      wayland_services_internal.empty() || wayland_window_api.empty() ||
      wayland_window_internal.empty() || wayland_window_configure.empty() ||
      wayland_registered_window_internal.empty() ||
      wayland_window_configure_internal.empty() ||
      wayland_window_events.empty() ||
      wayland_window_input_events.empty() ||
      wayland_window_drag_events.empty() ||
      wayland_window_text_events.empty() ||
      wayland_window_registered.empty() || wayland_window_bridge.empty() ||
      wayland_window_state_header.empty() || wayland_window_state.empty()) {
    return 77;
  }
  if (line_count(wayland_internal) > 80 ||
      !contains(wayland_internal, "#include \"wayland_input_internal.hpp\"") ||
      !contains(wayland_internal, "#include \"wayland_protocol_internal.hpp\"") ||
      !contains(wayland_internal, "#include \"wayland_services_internal.hpp\"") ||
      !contains(wayland_internal, "#include \"wayland_window_api.hpp\"") ||
      contains(wayland_internal, "struct xdg_wm_base_listener") ||
      contains(wayland_internal, "struct WaylandRegistryBindings") ||
      contains(wayland_internal, "create_wayland_window(") ||
      contains(wayland_internal, "create_wayland_text_input(")) {
    return 98;
  }
  if (!contains(wayland_protocol_internal,
                "extern const wl_interface xdg_wm_base_interface") ||
      !contains(wayland_protocol_internal,
                "struct zwp_text_input_v3_listener") ||
      !contains(wayland_protocol_internal,
                "xdg_surface_ack_configure(") ||
      !contains(wayland_protocol_internal,
                "zwp_text_input_v3_commit(")) {
    return 99;
  }
  if (line_count(wayland_protocol_xdg_interfaces) > 120 ||
      !contains(wayland_protocol_xdg_interfaces, "xdg_wm_base_interface") ||
      !contains(wayland_protocol_xdg_interfaces, "xdg_surface_interface") ||
      !contains(wayland_protocol_xdg_interfaces, "xdg_toplevel_interface") ||
      contains(wayland_protocol_xdg_interfaces, "xdg_surface_ack_configure(")) {
    return 113;
  }
  if (line_count(wayland_protocol_xdg_wm_base) > 70 ||
      !contains(wayland_protocol_xdg_wm_base,
                "xdg_wm_base_get_xdg_surface(") ||
      !contains(wayland_protocol_xdg_wm_base, "xdg_wm_base_pong(") ||
      contains(wayland_protocol_xdg_wm_base, "xdg_surface_ack_configure(") ||
      contains(wayland_protocol_xdg_wm_base, "xdg_toplevel_set_title(")) {
    return 114;
  }
  if (line_count(wayland_protocol_xdg_surface) > 70 ||
      !contains(wayland_protocol_xdg_surface,
                "xdg_surface_add_listener(") ||
      !contains(wayland_protocol_xdg_surface,
                "xdg_surface_ack_configure(") ||
      contains(wayland_protocol_xdg_surface, "xdg_wm_base_pong(") ||
      contains(wayland_protocol_xdg_surface, "xdg_toplevel_set_title(")) {
    return 115;
  }
  if (line_count(wayland_protocol_xdg_toplevel) > 60 ||
      !contains(wayland_protocol_xdg_toplevel,
                "xdg_toplevel_add_listener(") ||
      !contains(wayland_protocol_xdg_toplevel,
                "xdg_toplevel_set_title(") ||
      contains(wayland_protocol_xdg_toplevel, "xdg_surface_ack_configure(")) {
    return 116;
  }
  if (line_count(wayland_protocol_xdg) > 20 ||
      contains(wayland_protocol_xdg, "const wl_message xdg_toplevel_requests") ||
      contains(wayland_protocol_xdg, "const wl_interface xdg_wm_base_interface") ||
      contains(wayland_protocol_xdg, "xdg_surface_ack_configure(") ||
      contains(wayland_protocol_xdg, "xdg_toplevel_set_title(")) {
    return 114;
  }
  if (line_count(wayland_protocol_text_input_interfaces) > 80 ||
      !contains(wayland_protocol_text_input_interfaces,
                "zwp_text_input_manager_v3_interface") ||
      !contains(wayland_protocol_text_input_interfaces,
                "zwp_text_input_v3_interface") ||
      !contains(wayland_protocol_text_input_interfaces,
                "zwp_text_input_v3_events") ||
      contains(wayland_protocol_text_input_interfaces,
               "zwp_text_input_v3_commit(") ||
      contains(wayland_protocol_text_input_interfaces,
               "zwp_text_input_manager_v3_get_text_input(")) {
    return 117;
  }
  if (line_count(wayland_protocol_text_input_manager) > 50 ||
      !contains(wayland_protocol_text_input_manager,
                "zwp_text_input_manager_v3_destroy(") ||
      !contains(wayland_protocol_text_input_manager,
                "zwp_text_input_manager_v3_get_text_input(") ||
      contains(wayland_protocol_text_input_manager,
               "zwp_text_input_v3_enable(") ||
      contains(wayland_protocol_text_input_manager,
               "const wl_message zwp_text_input_v3_requests")) {
    return 118;
  }
  if (line_count(wayland_protocol_text_input_requests) > 110 ||
      !contains(wayland_protocol_text_input_requests,
                "zwp_text_input_v3_add_listener(") ||
      !contains(wayland_protocol_text_input_requests,
                "zwp_text_input_v3_enable(") ||
      !contains(wayland_protocol_text_input_requests,
                "zwp_text_input_v3_set_surrounding_text(") ||
      !contains(wayland_protocol_text_input_requests,
                "zwp_text_input_v3_set_cursor_rectangle(") ||
      !contains(wayland_protocol_text_input_requests,
                "zwp_text_input_v3_commit(") ||
      contains(wayland_protocol_text_input_requests,
               "zwp_text_input_manager_v3_get_text_input(") ||
      contains(wayland_protocol_text_input_requests,
               "const wl_message zwp_text_input_v3_events")) {
    return 119;
  }
  if (line_count(wayland_protocol_text_input) > 20 ||
      contains(wayland_protocol_text_input,
               "const wl_message zwp_text_input_v3_requests") ||
      contains(wayland_protocol_text_input,
               "const wl_interface zwp_text_input_v3_interface") ||
      contains(wayland_protocol_text_input,
               "zwp_text_input_manager_v3_get_text_input(") ||
      contains(wayland_protocol_text_input,
               "zwp_text_input_v3_commit(")) {
    return 120;
  }
  if (!contains(wayland_input_internal, "struct WaylandRegistryBindings") ||
      !contains(wayland_input_internal, "struct WaylandKeyboardState") ||
      !contains(wayland_input_internal,
                "struct WaylandCursorThemeState") ||
      !contains(wayland_input_internal,
                "wayland_bind_registry_global(") ||
      !contains(wayland_input_internal,
                "wayland_keyboard_load_keymap(")) {
    return 100;
  }
  if (!contains(wayland_window_api, "class WaylandWindow") ||
      !contains(wayland_window_api, "using WaylandWindowPtr") ||
      !contains(wayland_window_api, "create_wayland_window(") ||
      !contains(wayland_window_api, "wayland_window_pointer_moved(") ||
      !contains(wayland_window_api,
                "wayland_window_text_input_delete_surrounding(")) {
    return 101;
  }
  if (!contains(wayland_services_internal, "class WaylandTextInput") ||
      !contains(wayland_services_internal, "class WaylandDataDevice") ||
      !contains(wayland_services_internal,
                "create_wayland_atspi_accessibility_adapter(") ||
      !contains(wayland_services_internal,
                "create_wayland_native_menu_state(") ||
      !contains(wayland_services_internal,
                "wayland_show_native_file_dialog(")) {
    return 102;
  }

  const std::string wayland_text_input_internal =
      read_source("src/platform/linux/wayland_text_input_internal.hpp");
  const std::string wayland_text_input =
      read_source("src/platform/linux/wayland_text_input.cpp");
  const std::string wayland_text_input_core =
      read_source("src/platform/linux/wayland_text_input_core.cpp");
  const std::string wayland_text_input_events =
      read_source("src/platform/linux/wayland_text_input_events.cpp");
  const std::string wayland_text_input_requests =
      read_source("src/platform/linux/wayland_text_input_requests.cpp");
  if (wayland_text_input_internal.empty() || wayland_text_input.empty() ||
      wayland_text_input_core.empty() || wayland_text_input_events.empty() ||
      wayland_text_input_requests.empty()) {
    return 103;
  }
  if (!contains(wayland_text_input_internal, "class WaylandTextInput") ||
      !contains(wayland_text_input_internal, "handle_enter(") ||
      !contains(wayland_text_input_internal, "PendingDeleteSurroundingText") ||
      contains(wayland_text_input_internal, "zwp_text_input_v3_enable(")) {
    return 104;
  }
  if (line_count(wayland_text_input) > 100 ||
      contains(wayland_text_input, "class WaylandTextInput") ||
      contains(wayland_text_input, "zwp_text_input_v3_enable(") ||
      contains(wayland_text_input, "WaylandTextInput::handle_done(")) {
    return 105;
  }
  if (line_count(wayland_text_input_core) > 120 ||
      !contains(wayland_text_input_core, "WaylandTextInput::bind_to_seat(") ||
      !contains(wayland_text_input_core, "zwp_text_input_v3_add_listener") ||
      !contains(wayland_text_input_core, "WaylandTextInput::reset_text_input(") ||
      contains(wayland_text_input_core, "zwp_text_input_v3_enable(") ||
      contains(wayland_text_input_core, "WaylandTextInput::handle_done(")) {
    return 106;
  }
  if (line_count(wayland_text_input_events) > 150 ||
      !contains(wayland_text_input_events, "WaylandTextInput::handle_enter(") ||
      !contains(wayland_text_input_events, "WaylandTextInput::handle_leave(") ||
      !contains(wayland_text_input_events,
                "WaylandTextInput::handle_delete_surrounding_text(") ||
      !contains(wayland_text_input_events, "WaylandTextInput::handle_done(") ||
      contains(wayland_text_input_events, "zwp_text_input_v3_enable(")) {
    return 107;
  }
  if (line_count(wayland_text_input_requests) > 100 ||
      !contains(wayland_text_input_requests,
                "WaylandTextInput::apply_placement(") ||
      !contains(wayland_text_input_requests, "zwp_text_input_v3_enable(") ||
      !contains(wayland_text_input_requests,
                "zwp_text_input_v3_set_cursor_rectangle") ||
      !contains(wayland_text_input_requests, "zwp_text_input_v3_commit") ||
      contains(wayland_text_input_requests,
               "WaylandTextInput::handle_done(")) {
    return 108;
  }
  const std::string wayland_data_device_drag_actions =
      read_source("src/platform/linux/wayland_data_device_drag_actions.cpp");
  const std::string wayland_data_device_drag_events =
      read_source("src/platform/linux/wayland_data_device_drag_events.cpp");
  const std::string wayland_data_device_drag =
      read_source("src/platform/linux/wayland_data_device_drag.cpp");
  if (wayland_data_device_drag_actions.empty() ||
      wayland_data_device_drag_events.empty() ||
      wayland_data_device_drag.empty()) {
    return 121;
  }
  if (line_count(wayland_data_device_drag_actions) > 110 ||
      !contains(wayland_data_device_drag_actions,
                "WaylandDataDevice::negotiate_active_offer(") ||
      !contains(wayland_data_device_drag_actions,
                "WaylandDataDevice::finish_active_offer(") ||
      !contains(wayland_data_device_drag_actions,
                "WaylandDataDevice::preferred_drag_action(") ||
      !contains(wayland_data_device_drag_actions,
                "WaylandDataDevice::drag_action_from_wayland(") ||
      contains(wayland_data_device_drag_actions,
               "WaylandDataDevice::handle_enter(") ||
      contains(wayland_data_device_drag_actions,
               "wayland_window_drag_dropped(")) {
    return 122;
  }
  if (line_count(wayland_data_device_drag_events) > 90 ||
      !contains(wayland_data_device_drag_events,
                "WaylandDataDevice::handle_enter(") ||
      !contains(wayland_data_device_drag_events,
                "WaylandDataDevice::handle_leave(") ||
      !contains(wayland_data_device_drag_events,
                "WaylandDataDevice::handle_motion(") ||
      !contains(wayland_data_device_drag_events,
                "WaylandDataDevice::handle_drop(") ||
      contains(wayland_data_device_drag_events,
               "WaylandDataDevice::preferred_drag_action(") ||
      contains(wayland_data_device_drag_events,
               "WaylandDataDevice::data_offer_version(")) {
    return 123;
  }
  if (line_count(wayland_data_device_drag) > 20 ||
      contains(wayland_data_device_drag,
               "WaylandDataDevice::handle_enter(") ||
      contains(wayland_data_device_drag,
               "WaylandDataDevice::negotiate_active_offer(") ||
      contains(wayland_data_device_drag,
               "WaylandDataDevice::finish_active_offer(")) {
    return 124;
  }
  if (line_count(wayland_window) > 220) {
    return 78;
  }
  if (contains(wayland_window, "class WaylandWindow final") ||
      contains(wayland_window, "class RegisteredWaylandWindow final") ||
      contains(wayland_window, "class WaylandTextInputState") ||
      contains(wayland_window, "struct WaylandXdgToplevelState") ||
      contains(wayland_window, "struct WaylandXdgConfigureState") ||
      contains(wayland_window, "struct WaylandWindowChromeState") ||
      contains(wayland_window, "handle_surface_configure(") ||
      contains(wayland_window, "wayland_window_pointer_moved(") ||
      contains(wayland_window, "std::string cursor_name_for_shape(") ||
      contains(wayland_window,
               "WaylandXdgToplevelState parse_xdg_toplevel_states(")) {
    return 79;
  }
  if (!contains(wayland_window_internal, "class WaylandWindow final") ||
      !contains(wayland_window_internal,
                "#include \"wayland_registered_window_internal.hpp\"") ||
      !contains(wayland_window_internal,
                "#include \"wayland_window_configure_internal.hpp\"") ||
      !contains(wayland_registered_window_internal,
                "class RegisteredWaylandWindow final") ||
      !contains(wayland_window_configure_internal,
                "void record_toplevel_configure_state(") ||
      !contains(wayland_window_configure_internal,
                "handle_surface_configure(") ||
      !contains(wayland_window_internal, "void pointer_moved(")) {
    return 88;
  }
  if (line_count(wayland_window_internal) > 120 ||
      contains(wayland_window_internal,
               "class RegisteredWaylandWindow final") ||
      contains(wayland_window_internal,
               "void record_toplevel_configure_state(") ||
      contains(wayland_window_internal, "handle_surface_configure(")) {
    return 128;
  }
  if (!contains(wayland_window, "WaylandWindow::create(") ||
      !contains(wayland_window, "WaylandWindow::initialize(") ||
      !contains(wayland_window, "WaylandWindow::~WaylandWindow()")) {
    return 89;
  }
  if (!contains(wayland_window_configure,
                "WaylandWindow::handle_surface_configure(") ||
      !contains(wayland_window_configure,
                "WaylandWindow::record_toplevel_configure_state(") ||
      !contains(wayland_window_configure,
                "WaylandWindow::dispatch_configure_lifecycle_events(")) {
    return 90;
  }
  if (line_count(wayland_window_events) > 70 ||
      !contains(wayland_window_events, "WaylandWindow::set_cursor(") ||
      !contains(wayland_window_events, "WaylandWindow::cursor_shape() const") ||
      !contains(wayland_window_events, "WaylandWindow::configured() const") ||
      !contains(wayland_window_events, "WaylandWindow::wakeup_requested()") ||
      !contains(wayland_window_events, "WaylandWindow::focus_changed(") ||
      contains(wayland_window_events, "WaylandWindow::pointer_moved(") ||
      contains(wayland_window_events, "WaylandWindow::drag_entered(") ||
      contains(wayland_window_events, "WaylandWindow::text_input_commit(")) {
    return 91;
  }
  if (line_count(wayland_window_input_events) > 70 ||
      !contains(wayland_window_input_events,
                "WaylandWindow::pointer_moved(") ||
      !contains(wayland_window_input_events,
                "WaylandWindow::pointer_exited(") ||
      !contains(wayland_window_input_events,
                "WaylandWindow::pointer_button(") ||
      !contains(wayland_window_input_events,
                "WaylandWindow::pointer_scrolled(") ||
      !contains(wayland_window_input_events,
                "WaylandWindow::keyboard_key(") ||
      contains(wayland_window_input_events,
               "WaylandWindow::drag_entered(") ||
      contains(wayland_window_input_events,
               "WaylandWindow::text_input_commit(")) {
    return 125;
  }
  if (line_count(wayland_window_drag_events) > 80 ||
      !contains(wayland_window_drag_events,
                "WaylandWindow::drag_entered(") ||
      !contains(wayland_window_drag_events,
                "WaylandWindow::drag_updated(") ||
      !contains(wayland_window_drag_events,
                "WaylandWindow::drag_dropped(") ||
      !contains(wayland_window_drag_events,
                "WaylandWindow::drag_exited(") ||
      contains(wayland_window_drag_events,
               "WaylandWindow::pointer_moved(") ||
      contains(wayland_window_drag_events,
               "WaylandWindow::text_input_commit(")) {
    return 126;
  }
  if (line_count(wayland_window_text_events) > 100 ||
      !contains(wayland_window_text_events,
                "WaylandWindow::set_ime_text_input_placement(") ||
      !contains(wayland_window_text_events,
                "WaylandWindow::text_input_commit(") ||
      !contains(wayland_window_text_events,
                "WaylandWindow::text_input_delete_surrounding(") ||
      !contains(wayland_window_text_events,
                "WaylandWindow::sync_text_input_state(") ||
      contains(wayland_window_text_events,
               "WaylandWindow::pointer_moved(") ||
      contains(wayland_window_text_events,
               "WaylandWindow::drag_entered(")) {
    return 127;
  }
  if (!contains(wayland_window_registered,
                "RegisteredWaylandWindow::RegisteredWaylandWindow(") ||
      !contains(wayland_window_registered,
                "make_registered_wayland_window(")) {
    return 92;
  }
  if (!contains(wayland_window_bridge, "wayland_window_pointer_moved(") ||
      !contains(wayland_window_bridge, "wayland_window_pointer_exited(") ||
      !contains(wayland_window_bridge,
                "wayland_window_text_input_delete_surrounding(") ||
      !contains(wayland_window_bridge, "wayland_window_focus_changed(")) {
    return 93;
  }
  if (!contains(wayland_window_state_header,
                "class WaylandTextInputState") ||
      !contains(wayland_window_state_header,
                "struct WaylandXdgConfigureState") ||
      !contains(wayland_window_state_header,
                "struct WaylandWindowChromeState") ||
      !contains(wayland_window_state_header, "cursor_name_for_shape(") ||
      !contains(wayland_window_state_header, "parse_xdg_toplevel_states(")) {
    return 80;
  }
  if (!contains(wayland_window_state,
                "ImeComposition WaylandTextInputState::preedit(") ||
      !contains(wayland_window_state,
                "ImeComposition WaylandTextInputState::commit(") ||
      !contains(wayland_window_state, "parse_xdg_toplevel_states(") ||
      !contains(wayland_window_state, "cursor_name_for_shape(")) {
    return 81;
  }

  const std::string wayland_application =
      read_source("src/platform/linux/wayland_application.cpp");
  const std::string wayland_application_internal =
      read_source("src/platform/linux/wayland_application_internal.hpp");
  const std::string wayland_application_core_internal =
      read_source("src/platform/linux/wayland_application_core_internal.hpp");
  const std::string wayland_application_registry_internal =
      read_source(
          "src/platform/linux/wayland_application_registry_internal.hpp");
  const std::string wayland_application_input_internal =
      read_source("src/platform/linux/wayland_application_input_internal.hpp");
  const std::string wayland_application_cursor_internal =
      read_source("src/platform/linux/wayland_application_cursor_internal.hpp");
  const std::string wayland_application_input =
      read_source("src/platform/linux/wayland_application_input.cpp");
  const std::string wayland_application_seat =
      read_source("src/platform/linux/wayland_application_seat.cpp");
  const std::string wayland_application_keyboard =
      read_source("src/platform/linux/wayland_application_keyboard.cpp");
  const std::string wayland_keyboard =
      read_source("src/platform/linux/wayland_keyboard.cpp");
  const std::string wayland_application_pointer =
      read_source("src/platform/linux/wayland_application_pointer.cpp");
  const std::string wayland_application_pointer_scroll =
      read_source("src/platform/linux/wayland_application_pointer_scroll.cpp");
  const std::string wayland_application_cursor =
      read_source("src/platform/linux/wayland_application_cursor.cpp");
  const std::string wayland_application_windows =
      read_source("src/platform/linux/wayland_application_windows.cpp");
  const std::string wayland_application_window_registry =
      read_source(
          "src/platform/linux/wayland_application_window_registry.cpp");
  const std::string wayland_application_window_creation =
      read_source(
          "src/platform/linux/wayland_application_window_creation.cpp");
  const std::string wayland_application_lifecycle =
      read_source("src/platform/linux/wayland_application_lifecycle.cpp");
  const std::string wayland_application_services =
      read_source("src/platform/linux/wayland_application_services.cpp");
  const std::string wayland_font_discovery =
      read_source("src/platform/linux/wayland_font_discovery.cpp");
  const std::string wayland_application_factory =
      read_source("src/platform/linux/wayland_application_factory.cpp");
  if (wayland_application.empty() || wayland_application_internal.empty() ||
      wayland_application_core_internal.empty() ||
      wayland_application_registry_internal.empty() ||
      wayland_application_input_internal.empty() ||
      wayland_application_cursor_internal.empty() ||
      wayland_application_input.empty() || wayland_application_seat.empty() ||
      wayland_application_keyboard.empty() ||
      wayland_keyboard.empty() ||
      wayland_application_pointer.empty() ||
      wayland_application_pointer_scroll.empty() ||
      wayland_application_cursor.empty() ||
      wayland_application_windows.empty() ||
      wayland_application_window_registry.empty() ||
      wayland_application_window_creation.empty() ||
      wayland_application_lifecycle.empty() ||
      wayland_application_services.empty() ||
      wayland_font_discovery.empty() ||
      wayland_application_factory.empty()) {
    return 82;
  }
  if (line_count(wayland_application) > 140) {
    return 83;
  }
  if (contains(wayland_application, "class WaylandApplication final") ||
      contains(wayland_application,
               "WaylandApplication::create_window(") ||
      contains(wayland_application, "WaylandApplication::run(") ||
      contains(wayland_application, "WaylandApplication::request_wakeup(") ||
      contains(wayland_application, "WaylandApplication::quit(") ||
      contains(wayland_application,
               "WaylandApplication::install_native_menu(") ||
      contains(wayland_application,
               "WaylandApplication::show_native_file_dialog(") ||
      contains(wayland_application,
               "WaylandApplication::discover_font_discovery(") ||
      contains(wayland_application, "create_platform_application(") ||
      contains(wayland_application,
               "void WaylandApplication::handle_pointer_button(") ||
      contains(wayland_application,
               "void WaylandApplication::handle_keyboard_key(") ||
      contains(wayland_application,
               "void WaylandApplication::register_window(") ||
      contains(wayland_application,
               "void WaylandApplication::apply_cursor_for(")) {
    return 84;
  }
  if (!contains(wayland_application_internal,
                "#include \"wayland_application_core_internal.hpp\"") ||
      !contains(wayland_application_core_internal,
                "#include \"wayland_application_registry_internal.hpp\"") ||
      !contains(wayland_application_core_internal,
                "#include \"wayland_application_input_internal.hpp\"") ||
      !contains(wayland_application_core_internal,
                "#include \"wayland_application_cursor_internal.hpp\"") ||
      !contains(wayland_application_core_internal,
                "class WaylandApplication final") ||
      !contains(wayland_application_registry_internal, "handle_global(") ||
      !contains(wayland_application_registry_internal, "register_window(") ||
      !contains(wayland_application_input_internal,
                "handle_pointer_button(") ||
      !contains(wayland_application_input_internal,
                "handle_keyboard_key(") ||
      !contains(wayland_application_input_internal,
                "dispatch_pointer_scroll(") ||
      !contains(wayland_application_cursor_internal,
                "record_cursor_theme_state(") ||
      !contains(wayland_application_cursor_internal,
                "apply_cursor_for(")) {
    return 85;
  }
  if (line_count(wayland_application_internal) > 120 ||
      contains(wayland_application_internal,
               "class WaylandApplication final") ||
      contains(wayland_application_internal, "handle_pointer_button(") ||
      contains(wayland_application_internal, "handle_keyboard_key(") ||
      contains(wayland_application_internal, "register_window(") ||
      contains(wayland_application_internal, "apply_cursor_for(") ||
      contains(wayland_application_internal, "wl_display* display_")) {
    return 129;
  }
  if (line_count(wayland_application_input) > 80 ||
      contains(wayland_application_input,
               "WaylandApplication::handle_seat_capabilities(") ||
      contains(wayland_application_input,
               "WaylandApplication::handle_keyboard_key(") ||
      contains(wayland_application_input,
               "WaylandApplication::handle_pointer_button(") ||
      contains(wayland_application_input,
               "WaylandApplication::apply_cursor_for(")) {
    return 86;
  }
  if (!contains(wayland_application_seat,
                "WaylandApplication::handle_seat_capabilities(") ||
      !contains(wayland_application_seat,
                "wl_seat_get_pointer") ||
      !contains(wayland_application_seat,
                "wl_seat_get_keyboard") ||
      !contains(wayland_application_seat,
                "wl_pointer_release(") ||
      !contains(wayland_application_seat,
                "wl_keyboard_release(") ||
      !contains(wayland_application_seat,
                "wayland_window_focus_changed(") ||
      line_count(wayland_application_seat) > 120) {
    return 94;
  }
  if (!contains(wayland_application_keyboard,
                "WaylandApplication::handle_keyboard_key(") ||
      !contains(wayland_application_keyboard,
                "WaylandApplication::handle_keyboard_enter(") ||
      !contains(wayland_application_keyboard,
                "wayland_window_text_input(")) {
    return 95;
  }
  if (line_count(wayland_keyboard) > 145 ||
      !contains(wayland_keyboard, "release_xkb_resources(") ||
      !contains(wayland_keyboard, "apply_modifier_state(") ||
      !contains(wayland_keyboard, "keyboard.layout_group = group") ||
      contains(wayland_application_keyboard, "xkb_state_update_mask(")) {
    return 130;
  }
  if (line_count(wayland_application_pointer) > 80 ||
      !contains(wayland_application_pointer,
                "WaylandApplication::handle_pointer_enter(") ||
      !contains(wayland_application_pointer,
                "WaylandApplication::handle_pointer_leave(") ||
      !contains(wayland_application_pointer,
                "WaylandApplication::handle_pointer_button(") ||
      !contains(wayland_application_pointer,
                "WaylandApplication::handle_pointer_motion(") ||
      !contains(wayland_application_pointer,
                "wayland_window_pointer_moved(") ||
      !contains(wayland_application_pointer,
                "wayland_window_pointer_exited(") ||
      !contains(wayland_application_pointer, "pointer_enter_serial_ = 0") ||
      !contains(wayland_application_pointer, "pointer_scroll_frame_ = {}") ||
      contains(wayland_application_pointer,
               "WaylandApplication::handle_pointer_axis(") ||
      contains(wayland_application_pointer,
               "WaylandApplication::dispatch_pointer_scroll(")) {
    return 96;
  }
  if (line_count(wayland_application_pointer_scroll) > 110 ||
      !contains(wayland_application_pointer_scroll,
                "WaylandApplication::handle_pointer_axis(") ||
      !contains(wayland_application_pointer_scroll,
                "WaylandApplication::handle_pointer_frame(") ||
      !contains(wayland_application_pointer_scroll,
                "WaylandApplication::dispatch_pointer_scroll(") ||
      !contains(wayland_application_pointer_scroll,
                "wayland_window_pointer_scrolled(") ||
      !contains(wayland_application_pointer_scroll, "frame.precise") ||
      contains(wayland_application_pointer_scroll,
               "WaylandApplication::handle_pointer_button(")) {
    return 112;
  }
  if (!contains(wayland_application_cursor,
                "WaylandApplication::record_cursor_theme_state(")) {
    return 97;
  }
  if (!contains(wayland_application_window_registry,
                "WaylandApplication::register_window(") ||
      !contains(wayland_application_window_registry,
                "WaylandApplication::unregister_window(") ||
      !contains(wayland_application_window_registry,
                "WaylandApplication::handle_global(") ||
      !contains(wayland_application_window_registry,
                "WaylandApplication::find_window(")) {
    return 87;
  }
  if (!contains(wayland_application_window_creation,
                "WaylandApplication::create_window(") ||
      !contains(wayland_application_window_creation,
                "create_wayland_window(") ||
      !contains(wayland_application_window_creation,
                "make_registered_wayland_window(")) {
    return 130;
  }
  if (line_count(wayland_application_windows) > 100 ||
      contains(wayland_application_windows,
               "WaylandApplication::create_window(") ||
      contains(wayland_application_windows,
               "WaylandApplication::handle_global(") ||
      contains(wayland_application_windows,
               "WaylandApplication::register_window(") ||
      contains(wayland_application_windows,
               "WaylandApplication::find_window(")) {
    return 131;
  }
  if (!contains(wayland_application_lifecycle,
                "WaylandApplication::run(") ||
      !contains(wayland_application_lifecycle,
                "wayland_run_event_loop(") ||
      !contains(wayland_application_lifecycle,
                "WaylandApplication::request_wakeup(") ||
      !contains(wayland_application_lifecycle,
                "WaylandApplication::quit(")) {
    return 109;
  }
  if (!contains(wayland_application_services,
                "WaylandApplication::install_native_menu(") ||
      !contains(wayland_application_services,
                "WaylandApplication::show_native_file_dialog(") ||
      contains(wayland_application_services,
               "WaylandApplication::discover_font_discovery(") ||
      contains(wayland_application_services, "FontFaceDescriptor{") ||
      !contains(wayland_font_discovery,
                "WaylandApplication::discover_font_discovery(") ||
      !contains(wayland_font_discovery,
                "PlatformFontDiscoveryBackend::fontconfig") ||
      !contains(
          wayland_font_discovery,
          "PlatformFontDiscoveryStatus::deterministic_fallback") ||
      !contains(wayland_font_discovery, "fontconfig://sans-serif")) {
    return 110;
  }
  if (!contains(wayland_application_factory,
                "create_platform_application(") ||
      !contains(wayland_application_factory,
                "std::make_unique<WaylandApplication>()")) {
    return 111;
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
