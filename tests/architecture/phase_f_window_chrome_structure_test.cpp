#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  std::ifstream source(source_root() + "/" + path);
  if (!source) {
    source.open(path);
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
  std::size_t lines = 0;
  for (const char character : text) {
    lines += character == '\n' ? 1U : 0U;
  }
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string chrome_header = read_source(
      "include/cgpui/platform/platform_window_chrome.hpp");
  const std::string renderer_types =
      read_source("include/cgpui/renderer/renderer_types.hpp");
  const std::string chrome_default =
      read_source("src/platform/platform_window_chrome.cpp");
  const std::string empty = read_source("src/platform/empty.cpp");
  const std::string win32_chrome =
      read_source("src/platform/win32/win32_window_chrome.cpp");
  const std::string wayland_protocol = read_source(
      "src/platform/linux/wayland_protocol_xdg_decoration.cpp");
  const std::string wayland_interfaces = read_source(
      "src/platform/linux/wayland_protocol_xdg_decoration_interfaces.cpp");
  const std::string wayland_registry =
      read_source("src/platform/linux/wayland_registry.cpp");
  const std::string wayland_application = read_source(
      "src/platform/linux/wayland_application_core_internal.hpp");
  const std::string wayland_internal =
      read_source("src/platform/linux/wayland_window_internal.hpp");
  const std::string wayland_chrome =
      read_source("src/platform/linux/wayland_window_chrome.cpp");
  const std::string surface_selection = read_source(
      "src/renderer/vulkan/vulkan_surface_selection.cpp");
  const std::string swapchain_create = read_source(
      "src/renderer/vulkan/vulkan_swapchain_create.cpp");
  const std::string runtime_run = read_source("src/ui/runtime_run.cpp");
  const std::string runtime_activation =
      read_source("src/ui/runtime_window_activation.cpp");
  const std::string compositor =
      read_source("tests/platform/wayland_test_compositor.cpp");
  const std::string win32_test =
      read_source("tests/platform/win32_window_chrome_test.cpp");
  const std::string wayland_test =
      read_source("tests/platform/wayland_window_chrome_test.cpp");
  const std::string alpha_test = read_source(
      "tests/renderer/vulkan_transparent_composite_alpha_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &chrome_header, &renderer_types, &chrome_default, &empty, &win32_chrome,
      &wayland_protocol, &wayland_interfaces, &wayland_registry,
      &wayland_application, &wayland_internal, &wayland_chrome,
      &surface_selection, &swapchain_create, &runtime_run,
      &runtime_activation, &compositor, &win32_test, &wayland_test,
      &alpha_test, &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan,
      &findings};
  for (const std::string* source : required) {
    if (source->empty()) {
      return 1;
    }
  }
  if (!contains(chrome_header, "decoration_control_supported") ||
      !contains(chrome_header, "transparency_supported") ||
      !contains(renderer_types, "bool transparent_background = false;") ||
      !contains(chrome_default, "PlatformWindow::apply_window_chrome(") ||
      contains(empty, "PlatformWindow::apply_window_chrome(")) {
    return 2;
  }
  if (!contains(win32_chrome, "SetWindowLongPtrW") ||
      !contains(win32_chrome, "SetLayeredWindowAttributes") ||
      !contains(win32_chrome, "decoration_control_supported = true") ||
      !contains(win32_chrome, "transparency_supported = true") ||
      !contains(win32_test, "WS_EX_LAYERED") ||
      !contains(win32_test, "WS_OVERLAPPEDWINDOW")) {
    return 3;
  }
  if (!contains(wayland_interfaces, "zxdg_decoration_manager_v1") ||
      !contains(wayland_protocol, "get_toplevel_decoration") ||
      !contains(wayland_protocol, "zxdg_toplevel_decoration_v1_set_mode") ||
      !contains(wayland_registry, "zxdg_decoration_manager_v1_interface") ||
      !contains(wayland_application, "decoration_manager_") ||
      !contains(wayland_internal, "wayland_window_decoration_internal.hpp") ||
      !contains(wayland_chrome, "xdg_toplevel_set_min_size") ||
      !contains(wayland_chrome, "xdg_toplevel_set_max_size") ||
      !contains(wayland_chrome, "wl_display_flush(display_)")) {
    return 4;
  }
  if (!contains(surface_selection, "transparent_preference") ||
      !contains(surface_selection, "transparent_background") ||
      !contains(swapchain_create, "descriptor_.transparent_background") ||
      !contains(runtime_run, "descriptor.chrome.transparent_background") ||
      !contains(runtime_activation,
                "record.descriptor.chrome.transparent_background") ||
      !contains(alpha_test, "VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR")) {
    return 5;
  }
  if (!contains(compositor, "zxdg_decoration_manager_v1_interface") ||
      !contains(compositor, "set_decoration_mode") ||
      !contains(wayland_test,
                "wait_for_client_side_decoration_requested") ||
      !contains(wayland_test,
                "wait_for_server_side_decoration_requested") ||
      !contains(xmake, "target(\"win32_window_chrome_test\")") ||
      !contains(xmake, "target(\"wayland_window_chrome_test\")") ||
      !contains(xmake,
                "target(\"vulkan_transparent_composite_alpha_test\")") ||
      !contains(xmake,
                "target(\"phase_f_window_chrome_structure_test\")")) {
    return 6;
  }
  if (line_count(chrome_default) > 20 || line_count(win32_chrome) > 55 ||
      line_count(wayland_protocol) > 60 || line_count(wayland_interfaces) > 35 ||
      line_count(wayland_internal) > 120 || line_count(wayland_chrome) > 60 ||
      line_count(surface_selection) > 65) {
    return 7;
  }
  constexpr const char* completion =
      "Phase F Step 545 adds real Win32 decorated/frameless/layered chrome "
      "transitions, Wayland xdg-decoration server/client-side negotiation "
      "with non-resizable size constraints, and transparent-aware Vulkan "
      "composite-alpha selection. Step 546 child-window ownership production "
      "behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 8;
    }
  }
  if (!contains(ledger_json,
                "\"phase_f_step_545_remaining_gap\": \"Step 546 child-window")) {
    return 9;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 546 child-window")) {
    return 10;
  }
  return 0;
}
