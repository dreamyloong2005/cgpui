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
    if (character == '\n') {
      ++lines;
    }
  }
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string output_header =
      read_source("src/platform/linux/wayland_output_scale.hpp");
  const std::string output_source =
      read_source("src/platform/linux/wayland_output_scale.cpp");
  const std::string registry =
      read_source("src/platform/linux/wayland_registry.cpp");
  const std::string application_registry = read_source(
      "src/platform/linux/wayland_application_window_registry.cpp");
  const std::string application_creation = read_source(
      "src/platform/linux/wayland_application_window_creation.cpp");
  const std::string window_internal =
      read_source("src/platform/linux/wayland_window_internal.hpp");
  const std::string scale_internal = read_source(
      "src/platform/linux/wayland_window_scale_internal.hpp");
  const std::string scale_source =
      read_source("src/platform/linux/wayland_window_scale.cpp");
  const std::string configure =
      read_source("src/platform/linux/wayland_window_configure.cpp");
  const std::string win32_size =
      read_source("src/platform/win32/win32_window_size.cpp");
  const std::string win32_test = read_source(
      "tests/platform/win32_window_resize_scale_state_test.cpp");
  const std::string wayland_test = read_source(
      "tests/platform/wayland_window_resize_scale_state_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  if (output_header.empty() || output_source.empty() || registry.empty() ||
      application_registry.empty() || application_creation.empty() ||
      window_internal.empty() || scale_internal.empty() ||
      scale_source.empty() || configure.empty() || win32_size.empty() ||
      win32_test.empty() || wayland_test.empty() || xmake.empty() ||
      roadmap.empty() || ledger_md.empty() || ledger_json.empty() ||
      task_plan.empty() || findings.empty()) {
    return 1;
  }

  if (!contains(output_header, "class WaylandOutputScaleRegistry") ||
      !contains(output_source, "wl_output_add_listener") ||
      !contains(output_source, "WaylandOutputScaleRegistry::handle_scale") ||
      !contains(output_source,
                "record->owner->callback_(output, scale, true)")) {
    return 2;
  }
  if (!contains(registry, "interface_name == wl_output_interface.name") ||
      !contains(registry, "bindings.output_scales->bind") ||
      !contains(application_registry,
                "WaylandApplication::configure_output_scale_registry()") ||
      !contains(application_registry,
                "wayland_window_output_scale_changed")) {
    return 3;
  }
  if (!contains(application_creation, "output_scales_.scale_for(output)") ||
      !contains(scale_internal, "Size logical_size_") ||
      !contains(scale_internal, "std::vector<wl_output*> entered_outputs_") ||
      !contains(scale_internal, "handle_surface_enter") ||
      !contains(scale_internal, "handle_surface_leave")) {
    return 4;
  }
  if (!contains(scale_source, "wl_surface_set_buffer_scale(surface_, scale)") ||
      !contains(scale_source, "wl_surface_commit(surface_)") ||
      !contains(scale_source, "logical_size_.width * state_.scale.value") ||
      !contains(scale_source, "if (!present)") ||
      !contains(scale_source, "std::erase(window->entered_outputs_, output)") ||
      !contains(configure, "update_framebuffer_size()")) {
    return 5;
  }
  if (!contains(win32_size, "state_.framebuffer_size = Size{width, height}") ||
      !contains(win32_size, "state_.scale = DpiScale{dpi / 96.0F}") ||
      !contains(win32_size, "callback_(WindowResized")) {
    return 6;
  }
  if (!contains(win32_test, "observed_window->state()") ||
      !contains(wayland_test, "compositor.request_output_scale(2)") ||
      !contains(wayland_test, "last_surface_buffer_scale() != 2")) {
    return 7;
  }
  if (!contains(xmake, "target(\"win32_window_resize_scale_state_test\")") ||
      !contains(xmake, "target(\"wayland_window_resize_scale_state_test\")") ||
      !contains(xmake,
                "target(\"phase_f_window_resize_scale_structure_test\")")) {
    return 8;
  }
  if (line_count(output_header) > 70 || line_count(output_source) > 130 ||
      line_count(scale_internal) > 20 || line_count(scale_source) > 80 ||
      line_count(application_registry) > 110 || line_count(configure) > 95) {
    return 9;
  }

  constexpr const char* completion =
      "Phase F Step 541 makes resize and scale state-before-event observable "
      "on Win32, and adds production Wayland `wl_output` scale tracking, "
      "surface enter/leave handling, buffer-scale updates, "
      "logical-to-framebuffer conversion, and real dynamic scale/resize "
      "coverage. Step 542 close policy production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 10;
    }
  }
  if (!contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 542 close policy")) {
    return 11;
  }
  return 0;
}
