#include <cstdlib>
#include <fstream>
#include <iterator>
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

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t count = 0;
  for (const char value : text) {
    if (value == '\n') {
      count += 1;
    }
  }
  return count;
}

} // namespace

int main(int argc, char** argv) {
  if (argc > 0 && argv[0] != nullptr) {
    executable_directory = std::filesystem::absolute(argv[0]).parent_path();
  }

  const std::vector<const char*> wayland_files{
      "src/platform/linux/wayland_internal.hpp",
      "src/platform/linux/wayland_input_internal.hpp",
      "src/platform/linux/wayland_protocol_internal.hpp",
      "src/platform/linux/wayland_services_internal.hpp",
      "src/platform/linux/wayland_window_api.hpp",
      "src/platform/linux/wayland_protocol.cpp",
      "src/platform/linux/wayland_protocol_xdg_interfaces.cpp",
      "src/platform/linux/wayland_protocol_xdg_surface.cpp",
      "src/platform/linux/wayland_protocol_xdg_toplevel.cpp",
      "src/platform/linux/wayland_protocol_xdg_toplevel_display.cpp",
      "src/platform/linux/wayland_protocol_xdg_wm_base.cpp",
      "src/platform/linux/wayland_protocol_text_input_interfaces.cpp",
      "src/platform/linux/wayland_protocol_text_input_manager.cpp",
      "src/platform/linux/wayland_protocol_text_input_requests.cpp",
      "src/platform/linux/wayland_protocol_text_input.cpp",
      "src/platform/linux/wayland_protocol_xdg.cpp",
      "src/platform/linux/wayland_window_internal.hpp",
      "src/platform/linux/wayland_registered_window_internal.hpp",
      "src/platform/linux/wayland_window_configure_internal.hpp",
      "src/platform/linux/wayland_window_close_internal.hpp",
      "src/platform/linux/wayland_window_display_internal.hpp",
      "src/platform/linux/wayland_window_position_internal.hpp",
      "src/platform/linux/wayland_window_scale_internal.hpp",
      "src/platform/linux/wayland_window.cpp",
      "src/platform/linux/wayland_window_close.cpp",
      "src/platform/linux/wayland_window_configure.cpp",
      "src/platform/linux/wayland_window_display.cpp",
      "src/platform/linux/wayland_window_position.cpp",
      "src/platform/linux/wayland_window_scale.cpp",
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
      "src/platform/linux/wayland_data_device_drag_events.cpp",
      "src/platform/linux/wayland_data_device_drag.cpp",
      "src/platform/linux/wayland_data_device_payload.cpp",
      "src/platform/linux/wayland_accessibility.cpp",
      "src/platform/linux/wayland_native.cpp",
      "src/platform/linux/wayland_event_loop.cpp",
      "src/platform/linux/wayland_registry.cpp",
      "src/platform/linux/wayland_output_scale.hpp",
      "src/platform/linux/wayland_output_scale.cpp",
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
      "src/platform/linux/wayland_application_cursor.cpp",
      "src/platform/linux/wayland_application_windows.cpp",
      "src/platform/linux/wayland_application_window_registry.cpp",
      "src/platform/linux/wayland_application_window_creation.cpp",
      "src/platform/linux/wayland_application_lifecycle.cpp",
      "src/platform/linux/wayland_application_services.cpp",
      "src/platform/linux/wayland_font_discovery.cpp",
      "src/platform/linux/wayland_application_factory.cpp",
      "src/platform/linux/wayland_application.cpp",
  };
  for (const char* path : wayland_files) {
    if (read_source(path).empty()) {
      return 1;
    }
  }

  const std::vector<const char*> win32_files{
      "src/platform/win32/win32_internal.hpp",
      "src/platform/win32/win32_helpers.cpp",
      "src/platform/win32/win32_string.cpp",
      "src/platform/win32/win32_drag_drop_helpers.cpp",
      "src/platform/win32/win32_drag_drop_payload.cpp",
      "src/platform/win32/win32_drag_drop_ole_payload.cpp",
      "src/platform/win32/win32_input_helpers.cpp",
      "src/platform/win32/win32_font_discovery.cpp",
      "src/platform/win32/win32_application.cpp",
      "src/platform/win32/win32_window_ime.cpp",
      "src/platform/win32/win32_window_ime_placement.cpp",
      "src/platform/win32/win32_window_close.cpp",
      "src/platform/win32/win32_window_display_internal.hpp",
      "src/platform/win32/win32_window_display.cpp",
      "src/platform/win32/win32_window_position.cpp",
  };
  for (const char* path : win32_files) {
    if (read_source(path).empty()) {
      return 14;
    }
  }

  const std::vector<const char*> clipboard_files{
      "src/platform/clipboard_internal.hpp",
      "src/platform/clipboard_memory.cpp",
      "src/platform/clipboard_win32_internal.hpp",
      "src/platform/clipboard_win32_text.cpp",
      "src/platform/clipboard_win32_read.cpp",
      "src/platform/clipboard_win32_write.cpp",
      "src/platform/clipboard_win32.cpp",
      "src/platform/clipboard_wayland_internal.hpp",
      "src/platform/clipboard_wayland_connection.cpp",
      "src/platform/clipboard_wayland_dispatch.cpp",
      "src/platform/clipboard_wayland_offer.cpp",
      "src/platform/clipboard_wayland_read.cpp",
      "src/platform/clipboard_wayland_read_payload.cpp",
      "src/platform/clipboard_wayland_source.cpp",
      "src/platform/clipboard_wayland_source_events.cpp",
      "src/platform/clipboard_wayland_source_io.cpp",
      "src/platform/clipboard_wayland.cpp",
      "src/platform/clipboard.cpp",
  };
  for (const char* path : clipboard_files) {
    if (read_source(path).empty()) {
      return 18;
    }
  }

  const std::string clipboard_factory =
      read_source("src/platform/clipboard.cpp");
  if (line_count(clipboard_factory) > 80 ||
      contains(clipboard_factory, "class Win32Clipboard") ||
      contains(clipboard_factory, "WaylandClipboard::Connection") ||
      contains(clipboard_factory, "MemoryClipboard::read_text")) {
    return 19;
  }

  const std::string clipboard_memory =
      read_source("src/platform/clipboard_memory.cpp");
  const std::string clipboard_win32_internal =
      read_source("src/platform/clipboard_win32_internal.hpp");
  const std::string clipboard_win32_text =
      read_source("src/platform/clipboard_win32_text.cpp");
  const std::string clipboard_win32_read =
      read_source("src/platform/clipboard_win32_read.cpp");
  const std::string clipboard_win32_write =
      read_source("src/platform/clipboard_win32_write.cpp");
  const std::string clipboard_win32 =
      read_source("src/platform/clipboard_win32.cpp");
  const std::string clipboard_wayland_internal =
      read_source("src/platform/clipboard_wayland_internal.hpp");
  const std::string clipboard_wayland_connection =
      read_source("src/platform/clipboard_wayland_connection.cpp");
  const std::string clipboard_wayland_offer =
      read_source("src/platform/clipboard_wayland_offer.cpp");
  const std::string clipboard_wayland_read =
      read_source("src/platform/clipboard_wayland_read.cpp");
  const std::string clipboard_wayland_read_payload =
      read_source("src/platform/clipboard_wayland_read_payload.cpp");
  const std::string clipboard_wayland_dispatch =
      read_source("src/platform/clipboard_wayland_dispatch.cpp");
  const std::string clipboard_wayland_source =
      read_source("src/platform/clipboard_wayland_source.cpp");
  const std::string clipboard_wayland_source_events =
      read_source("src/platform/clipboard_wayland_source_events.cpp");
  const std::string clipboard_wayland_source_io =
      read_source("src/platform/clipboard_wayland_source_io.cpp");
  const std::string clipboard_wayland =
      read_source("src/platform/clipboard_wayland.cpp");
  if (!contains(clipboard_memory, "MemoryClipboard::read_text") ||
      !contains(clipboard_win32_internal, "class Win32Clipboard final") ||
      !contains(clipboard_win32_text, "widen_clipboard_text(") ||
      !contains(clipboard_win32_text, "narrow_clipboard_text(") ||
      !contains(clipboard_win32_read, "Win32Clipboard::read_text()") ||
      !contains(clipboard_win32_read, "GetClipboardData") ||
      !contains(clipboard_win32_write, "Win32Clipboard::write_text(") ||
      !contains(clipboard_win32_write, "SetClipboardData") ||
      !contains(clipboard_win32, "create_win32_clipboard") ||
      !contains(clipboard_wayland_internal,
                "struct WaylandClipboard::Connection") ||
      !contains(clipboard_wayland_connection,
                "Connection::create(std::string_view display_name)") ||
      !contains(clipboard_wayland_connection,
                "void WaylandClipboard::Connection::initialize()") ||
      !contains(clipboard_wayland_offer,
                "void WaylandClipboard::Connection::handle_data_offer(") ||
      !contains(clipboard_wayland_offer,
                "void WaylandClipboard::Connection::handle_selection(") ||
      !contains(clipboard_wayland_read,
                "std::optional<std::string> "
                "WaylandClipboard::Connection::read_text()") ||
      !contains(clipboard_wayland_read,
                "WaylandClipboard::Connection::preferred_text_mime_type()") ||
      !contains(clipboard_wayland_read_payload,
                "std::optional<std::string> "
                "WaylandClipboard::Connection::read_offer_payload(") ||
      !contains(clipboard_wayland_read_payload, "wl_data_offer_receive") ||
      !contains(clipboard_wayland_dispatch,
                "void WaylandClipboard::Connection::start_dispatch_thread()") ||
      !contains(clipboard_wayland_dispatch,
                "void WaylandClipboard::Connection::dispatch_owned_selection_events()") ||
      !contains(clipboard_wayland_source,
                "bool WaylandClipboard::Connection::write_text(") ||
      !contains(clipboard_wayland_source,
                "wl_data_source_add_listener") ||
      !contains(clipboard_wayland_source_events,
                "void WaylandClipboard::Connection::handle_source_send(") ||
      !contains(clipboard_wayland_source_io,
                "void WaylandClipboard::Connection::write_payload_to_fd(") ||
      !contains(clipboard_wayland, "create_wayland_clipboard")) {
    return 20;
  }
  if (line_count(clipboard_win32_internal) > 50 ||
      contains(clipboard_win32_internal, "GetClipboardData") ||
      contains(clipboard_win32_internal, "SetClipboardData") ||
      line_count(clipboard_win32_text) > 90 ||
      contains(clipboard_win32_text, "class Win32Clipboard") ||
      contains(clipboard_win32_text, "GetClipboardData") ||
      line_count(clipboard_win32_read) > 70 ||
      contains(clipboard_win32_read, "SetClipboardData") ||
      contains(clipboard_win32_read, "widen_clipboard_text(") ||
      line_count(clipboard_win32_write) > 80 ||
      contains(clipboard_win32_write, "GetClipboardData") ||
      contains(clipboard_win32_write, "narrow_clipboard_text(") ||
      line_count(clipboard_win32) > 40 ||
      contains(clipboard_win32, "class Win32Clipboard") ||
      contains(clipboard_win32, "GetClipboardData") ||
      contains(clipboard_win32, "SetClipboardData") ||
      contains(clipboard_win32, "widen_clipboard_text(") ||
      contains(clipboard_win32, "narrow_clipboard_text(")) {
    return 37;
  }
  if (line_count(clipboard_wayland_source) > 100 ||
      contains(clipboard_wayland_source,
               "dispatch_owned_selection_events(") ||
      contains(clipboard_wayland_source, "write_payload_to_fd(") ||
      contains(clipboard_wayland_source, "handle_source_send(") ||
      line_count(clipboard_wayland_dispatch) > 90 ||
      contains(clipboard_wayland_dispatch, "wl_data_source_add_listener") ||
      contains(clipboard_wayland_dispatch, "handle_source_send(") ||
      line_count(clipboard_wayland_source_events) > 100 ||
      contains(clipboard_wayland_source_events,
               "dispatch_owned_selection_events(") ||
      contains(clipboard_wayland_source_events, "while (written < payload.size())") ||
      line_count(clipboard_wayland_source_io) > 50 ||
      contains(clipboard_wayland_source_io, "handle_source_send(") ||
      contains(clipboard_wayland_source_io,
               "dispatch_owned_selection_events(")) {
    return 22;
  }
  if (line_count(clipboard_wayland) > 120 ||
      contains(clipboard_wayland, "struct WaylandClipboard::Connection") ||
      contains(clipboard_wayland, "wl_data_offer_receive") ||
      contains(clipboard_wayland, "wl_data_source_add_listener") ||
      contains(clipboard_wayland, "wl_display_connect")) {
    return 21;
  }
  if (line_count(clipboard_wayland_read) > 70 ||
      contains(clipboard_wayland_read, "handle_data_offer(") ||
      contains(clipboard_wayland_read, "wl_data_offer_receive") ||
      line_count(clipboard_wayland_offer) > 100 ||
      contains(clipboard_wayland_offer, "read_offer_payload(") ||
      contains(clipboard_wayland_offer, "wl_data_offer_receive") ||
      line_count(clipboard_wayland_read_payload) > 80 ||
      contains(clipboard_wayland_read_payload, "handle_data_offer(")) {
    return 31;
  }

  const std::string internal =
      read_source("src/platform/linux/wayland_internal.hpp");
  const std::string input_internal =
      read_source("src/platform/linux/wayland_input_internal.hpp");
  if (line_count(internal) > 80 ||
      !contains(internal, "#include \"wayland_input_internal.hpp\"") ||
      !contains(internal, "#include \"wayland_protocol_internal.hpp\"") ||
      !contains(internal, "#include \"wayland_services_internal.hpp\"") ||
      !contains(internal, "#include \"wayland_window_api.hpp\"")) {
    return 27;
  }
  if (!contains(input_internal, "struct WaylandCursorThemeState") ||
      !contains(input_internal, "WaylandCursorThemeLoadStatus") ||
      !contains(input_internal, "WaylandCursorImageState")) {
    return 5;
  }

  const std::string application =
      read_source("src/platform/linux/wayland_application.cpp");
  if (line_count(application) > 320) {
    return 2;
  }
  if (contains(application, "class WaylandApplication final") ||
      contains(application, "class WaylandWindow") ||
      contains(application, "class WaylandTextInput") ||
      contains(application, "class WaylandDataDevice") ||
      contains(application, "class WaylandAtspiAccessibilityAdapter") ||
      contains(application, "struct WaylandCursorThemeState") ||
      contains(application, "struct WaylandXdgConfigureState") ||
      contains(application,
               "void WaylandApplication::handle_pointer_button(") ||
      contains(application,
               "void WaylandApplication::handle_keyboard_key(") ||
      contains(application, "void WaylandApplication::register_window(") ||
      contains(application, "void WaylandApplication::apply_cursor_for(")) {
    return 3;
  }

  const std::string protocol =
      read_source("src/platform/linux/wayland_protocol.cpp");
  const std::string protocol_xdg_interfaces =
      read_source("src/platform/linux/wayland_protocol_xdg_interfaces.cpp");
  const std::string protocol_xdg_surface =
      read_source("src/platform/linux/wayland_protocol_xdg_surface.cpp");
  const std::string protocol_xdg_toplevel =
      read_source("src/platform/linux/wayland_protocol_xdg_toplevel.cpp");
  const std::string protocol_xdg_wm_base =
      read_source("src/platform/linux/wayland_protocol_xdg_wm_base.cpp");
  const std::string protocol_xdg =
      read_source("src/platform/linux/wayland_protocol_xdg.cpp");
  const std::string protocol_text_input_interfaces =
      read_source("src/platform/linux/wayland_protocol_text_input_interfaces.cpp");
  const std::string protocol_text_input_manager =
      read_source("src/platform/linux/wayland_protocol_text_input_manager.cpp");
  const std::string protocol_text_input_requests =
      read_source("src/platform/linux/wayland_protocol_text_input_requests.cpp");
  const std::string protocol_text_input =
      read_source("src/platform/linux/wayland_protocol_text_input.cpp");
  if (line_count(protocol) > 40 ||
      contains(protocol, "xdg_wm_base_interface") ||
      contains(protocol, "zwp_text_input_v3_interface")) {
    return 4;
  }
  if (line_count(protocol_xdg_interfaces) > 120 ||
      !contains(protocol_xdg_interfaces, "xdg_wm_base_interface") ||
      !contains(protocol_xdg_interfaces, "xdg_surface_interface") ||
      !contains(protocol_xdg_interfaces, "xdg_toplevel_interface") ||
      contains(protocol_xdg_interfaces, "xdg_surface_ack_configure(")) {
    return 32;
  }
  if (line_count(protocol_xdg_wm_base) > 70 ||
      !contains(protocol_xdg_wm_base, "xdg_wm_base_add_listener(") ||
      !contains(protocol_xdg_wm_base, "xdg_wm_base_get_xdg_surface(") ||
      !contains(protocol_xdg_wm_base, "xdg_wm_base_pong(") ||
      contains(protocol_xdg_wm_base, "xdg_surface_ack_configure(") ||
      contains(protocol_xdg_wm_base, "xdg_toplevel_set_title(")) {
    return 33;
  }
  if (line_count(protocol_xdg_surface) > 70 ||
      !contains(protocol_xdg_surface, "xdg_surface_add_listener(") ||
      !contains(protocol_xdg_surface, "xdg_surface_get_toplevel(") ||
      !contains(protocol_xdg_surface, "xdg_surface_ack_configure(") ||
      contains(protocol_xdg_surface, "xdg_wm_base_pong(") ||
      contains(protocol_xdg_surface, "xdg_toplevel_set_title(")) {
    return 34;
  }
  if (line_count(protocol_xdg_toplevel) > 60 ||
      !contains(protocol_xdg_toplevel, "xdg_toplevel_add_listener(") ||
      !contains(protocol_xdg_toplevel, "xdg_toplevel_set_title(") ||
      contains(protocol_xdg_toplevel, "xdg_surface_ack_configure(") ||
      contains(protocol_xdg_toplevel, "xdg_wm_base_pong(")) {
    return 35;
  }
  if (line_count(protocol_xdg) > 20 ||
      contains(protocol_xdg, "const wl_message xdg_toplevel_requests") ||
      contains(protocol_xdg, "const wl_interface xdg_wm_base_interface") ||
      contains(protocol_xdg, "xdg_surface_ack_configure(") ||
      contains(protocol_xdg, "xdg_toplevel_set_title(")) {
    return 28;
  }
  if (line_count(protocol_text_input_interfaces) > 80 ||
      !contains(protocol_text_input_interfaces,
                "zwp_text_input_manager_v3_interface") ||
      !contains(protocol_text_input_interfaces,
                "zwp_text_input_v3_interface") ||
      !contains(protocol_text_input_interfaces, "zwp_text_input_v3_events") ||
      contains(protocol_text_input_interfaces,
               "zwp_text_input_v3_commit(") ||
      line_count(protocol_text_input_manager) > 50 ||
      !contains(protocol_text_input_manager,
                "zwp_text_input_manager_v3_get_text_input(") ||
      contains(protocol_text_input_manager, "zwp_text_input_v3_enable(") ||
      line_count(protocol_text_input_requests) > 110 ||
      !contains(protocol_text_input_requests, "zwp_text_input_v3_commit(") ||
      !contains(protocol_text_input_requests, "zwp_text_input_v3_enable(") ||
      contains(protocol_text_input_requests,
               "zwp_text_input_manager_v3_get_text_input(") ||
      line_count(protocol_text_input) > 20 ||
      contains(protocol_text_input, "zwp_text_input_v3_commit(") ||
      contains(protocol_text_input, "zwp_text_input_v3_interface")) {
    return 29;
  }

  const std::string window_internal =
      read_source("src/platform/linux/wayland_window_internal.hpp");
  const std::string registered_window_internal =
      read_source("src/platform/linux/wayland_registered_window_internal.hpp");
  const std::string window_configure_internal =
      read_source("src/platform/linux/wayland_window_configure_internal.hpp");
  const std::string window = read_source("src/platform/linux/wayland_window.cpp");
  const std::string window_configure =
      read_source("src/platform/linux/wayland_window_configure.cpp");
  const std::string window_events =
      read_source("src/platform/linux/wayland_window_events.cpp");
  const std::string window_input_events =
      read_source("src/platform/linux/wayland_window_input_events.cpp");
  const std::string window_drag_events =
      read_source("src/platform/linux/wayland_window_drag_events.cpp");
  const std::string window_text_events =
      read_source("src/platform/linux/wayland_window_text_events.cpp");
  const std::string window_registered =
      read_source("src/platform/linux/wayland_window_registered.cpp");
  const std::string window_bridge =
      read_source("src/platform/linux/wayland_window_bridge.cpp");
  if (!contains(window_internal, "class WaylandWindow final") ||
      !contains(window_internal,
                "#include \"wayland_registered_window_internal.hpp\"") ||
      !contains(window_internal,
                "#include \"wayland_window_configure_internal.hpp\"") ||
      !contains(registered_window_internal,
                "class RegisteredWaylandWindow final") ||
      !contains(window_configure_internal,
                "void record_toplevel_configure_state(") ||
      !contains(window_configure_internal, "handle_surface_configure(") ||
      !contains(window, "WaylandWindow::create(") ||
      !contains(window, "WaylandWindow::initialize(") ||
      !contains(window_configure, "WaylandWindow::handle_surface_configure(") ||
      !contains(window_events, "WaylandWindow::set_cursor(") ||
      !contains(window_input_events, "WaylandWindow::pointer_moved(") ||
      !contains(window_drag_events, "WaylandWindow::drag_entered(") ||
      !contains(window_text_events, "WaylandWindow::text_input_commit(") ||
      !contains(window_text_events, "append_ime_default_preedit_style") ||
      !contains(window_text_events, "ImeDeleteSurroundingText") ||
      !contains(window_registered, "make_registered_wayland_window(") ||
      !contains(window_bridge, "wayland_window_pointer_moved(")) {
    return 10;
  }
  if (line_count(window_internal) > 120 ||
      contains(window_internal, "class RegisteredWaylandWindow final") ||
      contains(window_internal, "void record_toplevel_configure_state(") ||
      contains(window_internal, "handle_surface_configure(")) {
    return 38;
  }
  if (line_count(window_events) > 70 ||
      contains(window_events, "WaylandWindow::pointer_moved(") ||
      contains(window_events, "WaylandWindow::text_input_commit(") ||
      line_count(window_input_events) > 70 ||
      contains(window_input_events, "WaylandWindow::drag_entered(") ||
      line_count(window_drag_events) > 80 ||
      contains(window_drag_events, "WaylandWindow::text_input_commit(") ||
      line_count(window_text_events) > 100 ||
      contains(window_text_events, "WaylandWindow::pointer_moved(")) {
    return 36;
  }
  if (line_count(window) > 220 ||
      contains(window, "class WaylandWindow final") ||
      contains(window, "class RegisteredWaylandWindow final") ||
      contains(window, "wayland_window_pointer_moved(") ||
      contains(window, "WaylandWindow::handle_surface_configure(")) {
    return 25;
  }

  const std::string window_state_header =
      read_source("src/platform/linux/wayland_window_state.hpp");
  const std::string window_state =
      read_source("src/platform/linux/wayland_window_state.cpp");
  if (!contains(window_state_header, "class WaylandTextInputState") ||
      !contains(window_state_header, "struct WaylandXdgConfigureState") ||
      !contains(window_state_header, "struct WaylandWindowChromeState") ||
      !contains(window_state_header, "cursor_name_for_shape(") ||
      !contains(window_state, "parse_xdg_toplevel_states(") ||
      !contains(window_state, "cursor_name_for_shape(")) {
    return 21;
  }

  const std::string application_internal =
      read_source("src/platform/linux/wayland_application_internal.hpp");
  const std::string application_core_internal =
      read_source("src/platform/linux/wayland_application_core_internal.hpp");
  const std::string application_registry_internal =
      read_source("src/platform/linux/wayland_application_registry_internal.hpp");
  const std::string application_input_internal =
      read_source("src/platform/linux/wayland_application_input_internal.hpp");
  const std::string application_cursor_internal =
      read_source("src/platform/linux/wayland_application_cursor_internal.hpp");
  const std::string application_input =
      read_source("src/platform/linux/wayland_application_input.cpp");
  const std::string application_seat =
      read_source("src/platform/linux/wayland_application_seat.cpp");
  const std::string application_keyboard =
      read_source("src/platform/linux/wayland_application_keyboard.cpp");
  const std::string application_pointer =
      read_source("src/platform/linux/wayland_application_pointer.cpp");
  const std::string application_pointer_scroll =
      read_source("src/platform/linux/wayland_application_pointer_scroll.cpp");
  const std::string application_cursor =
      read_source("src/platform/linux/wayland_application_cursor.cpp");
  const std::string application_windows =
      read_source("src/platform/linux/wayland_application_windows.cpp");
  const std::string application_window_registry =
      read_source("src/platform/linux/wayland_application_window_registry.cpp");
  const std::string application_window_creation =
      read_source("src/platform/linux/wayland_application_window_creation.cpp");
  const std::string application_lifecycle =
      read_source("src/platform/linux/wayland_application_lifecycle.cpp");
  const std::string application_services =
      read_source("src/platform/linux/wayland_application_services.cpp");
  const std::string wayland_font_discovery =
      read_source("src/platform/linux/wayland_font_discovery.cpp");
  const std::string application_factory =
      read_source("src/platform/linux/wayland_application_factory.cpp");
  if (!contains(application_internal,
                "#include \"wayland_application_core_internal.hpp\"") ||
      !contains(application_core_internal,
                "#include \"wayland_application_registry_internal.hpp\"") ||
      !contains(application_core_internal,
                "#include \"wayland_application_input_internal.hpp\"") ||
      !contains(application_core_internal,
                "#include \"wayland_application_cursor_internal.hpp\"") ||
      !contains(application_core_internal, "class WaylandApplication final") ||
      !contains(application_registry_internal, "handle_global(") ||
      !contains(application_registry_internal, "register_window(") ||
      !contains(application_input_internal, "handle_pointer_button(") ||
      !contains(application_input_internal, "handle_keyboard_key(") ||
      !contains(application_input_internal, "dispatch_pointer_scroll(") ||
      !contains(application_cursor_internal, "record_cursor_theme_state(") ||
      !contains(application_cursor_internal, "apply_cursor_for(")) {
    return 22;
  }
  if (line_count(application_internal) > 120 ||
      contains(application_internal, "class WaylandApplication final") ||
      contains(application_internal, "handle_pointer_button(") ||
      contains(application_internal, "handle_keyboard_key(") ||
      contains(application_internal, "register_window(") ||
      contains(application_internal, "apply_cursor_for(") ||
      contains(application_internal, "wl_display* display_")) {
    return 39;
  }
  if (line_count(application_input) > 80 ||
      contains(application_input,
               "WaylandApplication::handle_pointer_button(") ||
      contains(application_input,
               "WaylandApplication::handle_keyboard_key(") ||
      contains(application_input,
               "WaylandApplication::dispatch_pointer_scroll(") ||
      contains(application_input,
               "WaylandApplication::apply_cursor_for(") ||
      !contains(application_seat,
                "WaylandApplication::handle_seat_capabilities(") ||
      !contains(application_keyboard,
                "WaylandApplication::handle_keyboard_key(") ||
      line_count(application_pointer) > 80 ||
      !contains(application_pointer,
                "WaylandApplication::handle_pointer_button(") ||
      !contains(application_pointer,
                "WaylandApplication::handle_pointer_motion(") ||
      contains(application_pointer,
               "WaylandApplication::handle_pointer_axis(") ||
      contains(application_pointer,
               "WaylandApplication::dispatch_pointer_scroll(") ||
      line_count(application_pointer_scroll) > 110 ||
      !contains(application_pointer_scroll,
                "WaylandApplication::handle_pointer_axis(") ||
      !contains(application_pointer_scroll,
                "WaylandApplication::dispatch_pointer_scroll(") ||
      !contains(application_pointer_scroll,
                "wayland_window_pointer_scrolled(") ||
      contains(application_pointer_scroll,
               "WaylandApplication::handle_pointer_button(") ||
      !contains(application_pointer,
                "wayland_window_pointer_button(") ||
      !contains(application_cursor,
                "WaylandApplication::record_cursor_theme_state(") ||
      !contains(application_cursor,
                "WaylandApplication::apply_cursor_for(") ||
      line_count(application) > 140 ||
      contains(application, "WaylandApplication::create_window(") ||
      contains(application, "WaylandApplication::run(") ||
      contains(application, "WaylandApplication::install_native_menu(") ||
      contains(application, "WaylandApplication::discover_font_discovery(") ||
      contains(application, "create_platform_application(")) {
    return 23;
  }
  if (!contains(application_window_registry,
                "WaylandApplication::register_window(") ||
      !contains(application_window_registry,
                "WaylandApplication::unregister_window(") ||
      !contains(application_window_registry,
                "WaylandApplication::handle_global(") ||
      !contains(application_window_registry,
                "WaylandApplication::find_window(")) {
    return 24;
  }
  if (!contains(application_window_creation,
                "WaylandApplication::create_window(") ||
      !contains(application_window_creation, "create_wayland_window(") ||
      !contains(application_window_creation,
                "make_registered_wayland_window(")) {
    return 40;
  }
  if (line_count(application_windows) > 100 ||
      contains(application_windows, "WaylandApplication::create_window(") ||
      contains(application_windows, "WaylandApplication::handle_global(") ||
      contains(application_windows, "WaylandApplication::register_window(") ||
      contains(application_windows, "WaylandApplication::find_window(")) {
    return 41;
  }
  if (!contains(application_lifecycle, "WaylandApplication::run(") ||
      !contains(application_lifecycle, "wayland_run_event_loop(") ||
      !contains(application_lifecycle,
                "WaylandApplication::request_wakeup(") ||
      !contains(application_lifecycle, "WaylandApplication::quit(") ||
      !contains(application_services,
                "WaylandApplication::install_native_menu(") ||
      !contains(application_services,
                "WaylandApplication::show_native_file_dialog(") ||
      contains(application_services,
               "WaylandApplication::discover_font_discovery(") ||
      !contains(wayland_font_discovery,
                "WaylandApplication::discover_font_discovery(") ||
      !contains(wayland_font_discovery,
                "PlatformFontDiscoveryBackend::fontconfig") ||
      !contains(application_factory, "create_platform_application(")) {
    return 30;
  }

  const std::string text_input_internal =
      read_source("src/platform/linux/wayland_text_input_internal.hpp");
  const std::string text_input =
      read_source("src/platform/linux/wayland_text_input.cpp");
  const std::string text_input_core =
      read_source("src/platform/linux/wayland_text_input_core.cpp");
  const std::string text_input_events =
      read_source("src/platform/linux/wayland_text_input_events.cpp");
  const std::string text_input_requests =
      read_source("src/platform/linux/wayland_text_input_requests.cpp");
  if (!contains(text_input_internal, "class WaylandTextInput") ||
      !contains(text_input_core, "WaylandTextInput::bind_to_seat(") ||
      !contains(text_input_events, "wayland_window_text_input_preedit") ||
      !contains(text_input_events, "wayland_window_text_input_commit") ||
      contains(text_input_events, "(void)serial") ||
      contains(text_input_events, "(void)cursor_begin") ||
      contains(text_input_events, "(void)cursor_end") ||
      !contains(text_input_events, "preedit.cursor_begin") ||
      !contains(text_input_events, "preedit.cursor_end") ||
      !contains(text_input_events, "text_input_done_serial_is_stale") ||
      !contains(text_input_events, "last_done_serial_ = serial") ||
      !contains(text_input_events, "reset_pending_events()") ||
      !contains(text_input_internal, "last_done_serial_") ||
      !contains(text_input_internal, "reset_pending_events()") ||
      !contains(text_input_core, "last_done_serial_ = 0") ||
      !contains(text_input_requests, "zwp_text_input_v3_commit") ||
      !contains(text_input_requests, "placement->surrounding_text") ||
      !contains(text_input_requests, "placement->selection_anchor") ||
      !contains(text_input_requests, "placement->content_hint") ||
      !contains(text_input_requests, "placement->candidate_rect") ||
      !contains(text_input, "create_wayland_text_input") ||
      contains(text_input, "class WaylandTextInput")) {
    return 6;
  }

  const std::string data_device =
      read_source("src/platform/linux/wayland_data_device.cpp");
  const std::string data_device_internal =
      read_source("src/platform/linux/wayland_data_device_internal.hpp");
  const std::string data_device_offer =
      read_source("src/platform/linux/wayland_data_device_offer.cpp");
  const std::string data_device_drag_actions =
      read_source("src/platform/linux/wayland_data_device_drag_actions.cpp");
  const std::string data_device_drag_events =
      read_source("src/platform/linux/wayland_data_device_drag_events.cpp");
  const std::string data_device_drag =
      read_source("src/platform/linux/wayland_data_device_drag.cpp");
  const std::string data_device_payload =
      read_source("src/platform/linux/wayland_data_device_payload.cpp");
  if (!contains(data_device_internal, "class WaylandDataDevice") ||
      !contains(data_device, "WaylandDataDevice::bind_to_seat(") ||
      !contains(data_device_offer,
                "WaylandDataDevice::handle_data_offer(") ||
      !contains(data_device_drag_events,
                "WaylandDataDevice::handle_enter(") ||
      !contains(data_device_drag_events, "wayland_window_drag_entered") ||
      !contains(data_device_drag_actions,
                "WaylandDataDevice::negotiate_active_offer(") ||
      !contains(data_device_payload,
                "WaylandDataDevice::read_offer_payload(") ||
      !contains(data_device_payload, "wl_data_offer_receive")) {
    return 7;
  }
  if (line_count(data_device) > 160 ||
      contains(data_device, "class WaylandDataDevice") ||
      contains(data_device, "wl_data_offer_receive") ||
      contains(data_device, "wayland_window_drag_entered") ||
      line_count(data_device_drag) > 20 ||
      contains(data_device_drag, "WaylandDataDevice::handle_enter(") ||
      contains(data_device_drag, "WaylandDataDevice::negotiate_active_offer(") ||
      line_count(data_device_drag_actions) > 110 ||
      contains(data_device_drag_actions,
               "WaylandDataDevice::handle_enter(") ||
      line_count(data_device_drag_events) > 90 ||
      contains(data_device_drag_events,
               "WaylandDataDevice::preferred_drag_action(")) {
    return 26;
  }

  const std::string accessibility =
      read_source("src/platform/linux/wayland_accessibility.cpp");
  if (!contains(accessibility, "class WaylandAtspiAccessibilityAdapter") ||
      !contains(accessibility, "WaylandAtspiObjectNode") ||
      !contains(accessibility, "PlatformAccessibilityTreeUpdate")) {
    return 8;
  }

  const std::string native =
      read_source("src/platform/linux/wayland_native.cpp");
  if (!contains(native, "WaylandNativeMenuState") ||
      !contains(native, "WaylandNativeFileDialogState") ||
      !contains(native, "backend = \"wayland\"")) {
    return 9;
  }

  const std::string event_loop =
      read_source("src/platform/linux/wayland_event_loop.cpp");
  if (!contains(event_loop, "wayland_run_event_loop") ||
      !contains(event_loop, "poll(fds.data()") ||
      !contains(event_loop, "wayland_window_wakeup_requested")) {
    return 11;
  }

  const std::string registry =
      read_source("src/platform/linux/wayland_registry.cpp");
  if (!contains(registry, "wayland_bind_registry_global") ||
      !contains(registry, "wl_registry_bind") ||
      !contains(registry, "zwp_text_input_manager_v3_interface")) {
    return 12;
  }

  const std::string keyboard =
      read_source("src/platform/linux/wayland_keyboard.cpp");
  if (!contains(keyboard, "wayland_keyboard_load_keymap") ||
      !contains(keyboard, "xkb_state_update_mask") ||
      !contains(keyboard, "xkb_state_key_get_utf8")) {
    return 13;
  }

  const std::string win32_application =
      read_source("src/platform/win32/win32_application.cpp");
  if (line_count(win32_application) > 920) {
    return 15;
  }
  if (contains(win32_application, "std::wstring widen(") ||
      contains(win32_application, "KeyboardModifiers current_modifiers()") ||
      contains(
          win32_application,
          "DragDropPayload drag_payload_from_ole_data_object(")) {
    return 16;
  }

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
  const std::string win32_font_discovery =
      read_source("src/platform/win32/win32_font_discovery.cpp");
  const std::string win32_window_ime =
      read_source("src/platform/win32/win32_window_ime.cpp");
  const std::string win32_window_ime_placement =
      read_source("src/platform/win32/win32_window_ime_placement.cpp");
  const std::string win32_window_proc_lifecycle =
      read_source("src/platform/win32/win32_window_proc_lifecycle.cpp");
  if (line_count(win32_helpers) > 60 ||
      contains(win32_helpers, "std::wstring widen(") ||
      contains(win32_helpers, "KeyboardModifiers current_modifiers()") ||
      contains(win32_helpers,
               "DragDropPayload drag_payload_from_ole_data_object(") ||
      !contains(win32_string, "std::wstring widen(") ||
      !contains(win32_string, "std::string utf8_from_utf16(") ||
      contains(win32_string, "drag_payload_from_ole_data_object(") ||
      line_count(win32_drag_drop_helpers) > 90 ||
      !contains(win32_drag_drop_helpers, "choose_ole_drop_effect(") ||
      !contains(win32_drag_drop_helpers, "test_drag_enter_message(") ||
      !contains(win32_drag_drop_helpers,
                "DragDropAction drag_action_from_test_hook(") ||
      contains(win32_drag_drop_helpers,
               "DragDropPayload drag_payload_from_test_hook(") ||
      contains(win32_drag_drop_helpers,
               "DragDropPayload drag_payload_from_ole_data_object(") ||
      line_count(win32_drag_drop_payload) > 50 ||
      !contains(win32_drag_drop_payload,
                "DragDropPayload drag_payload_from_test_hook(") ||
      contains(win32_drag_drop_payload,
                "DragDropPayload drag_payload_from_ole_data_object(") ||
      line_count(win32_drag_drop_ole_payload) > 90 ||
      !contains(win32_drag_drop_ole_payload,
                "DragDropPayload drag_payload_from_ole_data_object(") ||
      !contains(win32_drag_drop_ole_payload, "CF_HDROP") ||
      contains(win32_drag_drop_helpers, "KeyboardModifiers current_modifiers()") ||
      !contains(win32_input_helpers, "KeyboardModifiers current_modifiers()") ||
      !contains(win32_input_helpers, "const wchar_t* cursor_id_for(") ||
      !contains(win32_input_helpers, "win32_window_style_for(") ||
      contains(win32_input_helpers,
               "DragDropPayload drag_payload_from_ole_data_object(")) {
    return 17;
  }
  if (line_count(win32_window_ime) > 90 ||
      line_count(win32_window_ime_placement) > 90 ||
      !contains(win32_window_ime, "Win32Window::ime_composition(") ||
      !contains(win32_window_ime, "ImmGetCompositionStringW") ||
      !contains(win32_window_ime, "GCS_COMPSTR") ||
      !contains(win32_window_ime, "GCS_RESULTSTR") ||
      !contains(win32_window_ime, "ImeCompositionPhase::update") ||
      !contains(win32_window_ime, "ImeCompositionPhase::commit") ||
      !contains(win32_window_ime, "ImeCompositionPhase::cancel") ||
      contains(win32_window_ime, "ImmSetCandidateWindow") ||
      !contains(win32_window_ime_placement, "placement.candidate_rect") ||
      !contains(win32_window_ime_placement, "candidate_rect") ||
      !contains(win32_window_ime_placement, "ImmSetCandidateWindow") ||
      !contains(win32_window_ime_placement, "ImmSetCompositionWindow") ||
      contains(win32_window_ime_placement, "GCS_COMPSTR") ||
      contains(win32_window_ime_placement, "GCS_RESULTSTR") ||
      !contains(win32_window_proc_lifecycle, "WM_IME_COMPOSITION") ||
      !contains(win32_window_proc_lifecycle,
                "window->ime_composition(lparam)") ||
      contains(win32_window_proc_lifecycle, "ImmGetCompositionStringW")) {
    return 43;
  }
  if (!contains(win32_font_discovery, "win32_discover_fonts()") ||
      !contains(
          win32_font_discovery,
          "PlatformFontDiscoveryBackend::direct_write") ||
      !contains(
          win32_font_discovery,
          "PlatformFontDiscoveryStatus::deterministic_fallback") ||
      contains(win32_application, "FontFaceDescriptor{")) {
    return 42;
  }

  return 0;
}
