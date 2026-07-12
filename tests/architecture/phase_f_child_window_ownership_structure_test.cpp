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
  const std::string platform_application = read_source(
      "include/cgpui/platform/platform_application.hpp");
  const std::string platform_default =
      read_source("src/platform/platform_child_window.cpp");
  const std::string runtime_options = read_source(
      "include/cgpui/ui/runtime_window_options.hpp");
  const std::string runtime_windows =
      read_source("src/ui/runtime_windows.cpp");
  const std::string runtime_activation =
      read_source("src/ui/runtime_window_activation.cpp");
  const std::string runtime_ownership =
      read_source("src/ui/runtime_window_ownership.cpp");
  const std::string runtime_run = read_source("src/ui/runtime_run.cpp");
  const std::string win32_application =
      read_source("src/platform/win32/win32_application.cpp");
  const std::string win32_factory =
      read_source("src/platform/win32/win32_window_factory.cpp");
  const std::string wayland_child = read_source(
      "src/platform/linux/wayland_application_child_window.cpp");
  const std::string wayland_creation = read_source(
      "src/platform/linux/wayland_application_window_creation.cpp");
  const std::string wayland_protocol = read_source(
      "src/platform/linux/wayland_protocol_xdg_toplevel_parent.cpp");
  const std::string wayland_window =
      read_source("src/platform/linux/wayland_window.cpp");
  const std::string wayland_internal = read_source(
      "src/platform/linux/wayland_window_internal.hpp");
  const std::string runtime_test = read_source(
      "tests/ui/window_runtime_child_ownership_test.cpp");
  const std::string win32_test = read_source(
      "tests/platform/win32_child_window_ownership_test.cpp");
  const std::string wayland_test = read_source(
      "tests/platform/wayland_child_window_ownership_test.cpp");
  const std::string compositor =
      read_source("tests/platform/wayland_test_compositor.cpp");
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
      &platform_application, &platform_default, &runtime_options,
      &runtime_windows, &runtime_activation, &runtime_ownership, &runtime_run,
      &win32_application, &win32_factory, &wayland_child, &wayland_creation,
      &wayland_protocol, &wayland_window, &wayland_internal, &runtime_test,
      &win32_test, &wayland_test, &compositor, &xmake, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const std::string* source : required) {
    if (source->empty()) {
      return 1;
    }
  }
  if (!contains(platform_application, "create_child_window(") ||
      !contains(platform_default, "return create_window(") ||
      !contains(runtime_options, "WindowRuntimeId parent_runtime_id;") ||
      !contains(runtime_windows, ".parent_runtime_id = root_window_runtime_id_") ||
      !contains(runtime_activation, "application_.create_child_window(") ||
      !contains(runtime_ownership, "activate_pending_native_windows(") ||
      !contains(runtime_run, "activate_pending_native_windows();")) {
    return 2;
  }
  if (!contains(win32_application, "create_window_with_owner(") ||
      !contains(win32_factory, "CreateWindowExW(") ||
      !contains(win32_factory, "owner, nullptr, instance") ||
      !contains(win32_test, "GetWindow(child_hwnd, GW_OWNER)")) {
    return 3;
  }
  if (!contains(wayland_child, "RegisteredWaylandWindow") ||
      !contains(wayland_child, ".toplevel()") ||
      !contains(wayland_protocol, "xdg_toplevel_set_parent(") ||
      !contains(wayland_window, "xdg_toplevel_set_parent(toplevel_, parent)") ||
      !contains(compositor, "parent_requested.store(true)") ||
      !contains(wayland_test, "wait_for_parent_requested()")) {
    return 4;
  }
  if (!contains(runtime_test, "creation_order ==") ||
      !contains(runtime_test, "received_root_parent") ||
      !contains(xmake, "target(\"win32_child_window_ownership_test\")") ||
      !contains(xmake, "target(\"wayland_child_window_ownership_test\")") ||
      !contains(xmake, "target(\"window_runtime_child_ownership_test\")") ||
      !contains(xmake,
                "target(\"phase_f_child_window_ownership_structure_test\")")) {
    return 5;
  }
  if (line_count(platform_default) > 20 ||
      line_count(runtime_activation) > 75 ||
      line_count(runtime_ownership) > 25 || line_count(win32_factory) > 70 ||
      line_count(wayland_child) > 30 || line_count(wayland_protocol) > 15 ||
      line_count(wayland_internal) > 120 || line_count(runtime_test) > 130) {
    return 6;
  }
  constexpr const char* completion =
      "Phase F Step 546 adds root-owned runtime child windows with deferred "
      "activation after root creation, real Win32 owner HWNDs, and Wayland "
      "xdg-toplevel parent requests before first commit. Step 547 Win32 "
      "pointer input production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 7;
    }
  }
  if (!contains(ledger_json,
                "\"phase_f_step_546_remaining_gap\": \"Step 547 Win32")) {
    return 8;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 635 async task pool production depth")) {
    return 9;
  }
  return 0;
}
