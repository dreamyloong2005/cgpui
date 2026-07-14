#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string header = read_source("include/cgpui/ui/test_app.hpp");
  const std::string internal = read_source("src/ui/test_app_internal.hpp");
  const std::string input = read_source("src/ui/test_app_input.cpp");
  const std::string platform = read_source("src/ui/test_app_platform.cpp");
  const std::string window = read_source("src/ui/window.cpp");
  const std::string behavior =
      read_source("tests/ui/test_app_simulated_input_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_test_app_window_setup_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &header, &internal, &input, &platform, &window, &behavior, &previous,
      &xmake, &vocabulary, &core, &roadmap, &ledger_md, &ledger_json,
      &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "simulate_keystrokes(std::string_view keystrokes)") ||
      !contains(header, "dispatch_pointer_button(") ||
      !contains(header, "dispatch_window_activation(bool active)") ||
      !contains(header, "void focus(ElementId element_id) const")) return 2;
  if (!contains(internal, "bool dispatch_event(") ||
      !contains(platform, "TestPlatformWindow::dispatch_event(") ||
      !contains(input, "TestAppWindow::simulate_keystrokes(") ||
      !contains(input, "state_->dispatch_event(runtime_id_") ||
      !contains(input, "request_keyboard_focus_for_window")) return 3;
  if (!contains(window, "runtime_record->input") ||
      contains(window, "return runtime_->input_state()") ||
      !contains(behavior, "ctrl-k ctrl-s") ||
      !contains(behavior, "ctrl-unknown") ||
      !contains(behavior, "Second Input Window") ||
      !contains(behavior, "second.input_state().pointer_position") ||
      contains(behavior, "[DEBUG-step660]")) return 4;
  if (!contains(previous, "Step 676 candidate-ledger closeout audit") ||
      !contains(xmake, "target(\"phase_g_test_app_simulated_input_test\")") ||
      !contains(xmake,
                "target(\"phase_g_test_app_simulated_input_structure_test\")")) {
    return 5;
  }
  if (line_count(header) > 125 || line_count(internal) > 140 ||
      line_count(input) > 120 || line_count(platform) > 75 ||
      line_count(window) > 80 || line_count(behavior) > 125) return 6;

  constexpr const char* completion =
      "Phase G Step 660 routes GPUI-style keyboard, key-sequence, pointer, "
      "scroll, activation, window-focus, and element-focus simulation through "
      "TestAppWindow and the private test-platform callback, with window-scoped "
      "input snapshots and invalid grammar rejection. Step 661 GPUI-style "
      "timer control production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 7;
  }
  if (!contains(vocabulary, "`TestAppWindow::simulate_keystrokes(...)`") ||
      !contains(core, "Window-scoped `TestAppWindow` input simulation") ||
      !contains(ledger_json, "\"phase_g_step_660_sources\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 676 candidate-ledger closeout audit\"")) return 8;
  return 0;
}
