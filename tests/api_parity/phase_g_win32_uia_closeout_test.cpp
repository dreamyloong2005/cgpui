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

} // namespace

int main() {
  const std::string provider = read_source(
      "tests/architecture/phase_g_win32_uia_provider_object_structure_test.cpp");
  const std::string navigation = read_source(
      "tests/architecture/phase_g_win32_uia_tree_navigation_structure_test.cpp");
  const std::string patterns = read_source(
      "tests/architecture/phase_g_win32_uia_pattern_provider_structure_test.cpp");
  const std::string events = read_source(
      "tests/architecture/phase_g_win32_uia_live_event_structure_test.cpp");
  const std::string integration = read_source(
      "tests/architecture/phase_g_win32_uia_change_integration_structure_test.cpp");
  const std::string lifetime = read_source(
      "tests/architecture/phase_g_win32_uia_provider_lifetime_structure_test.cpp");
  const std::string stress = read_source(
      "tests/architecture/phase_g_win32_uia_lifecycle_stress_structure_test.cpp");
  const std::string source_guard = read_source(
      "tests/architecture/win32_window_source_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &provider, &navigation, &patterns, &events, &integration, &lifetime,
      &stress, &source_guard, &xmake, &roadmap, &ledger_md, &ledger_json,
      &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(provider, "IRawElementProviderSimple") ||
      !contains(navigation, "IRawElementProviderFragmentRoot") ||
      !contains(patterns, "IRangeValueProvider") ||
      !contains(events, "UIA_Text_TextChangedEventId") ||
      !contains(integration, "request_keyboard_focus(integration_input_id)") ||
      !contains(lifetime, "UIA_E_ELEMENTNOTAVAILABLE") ||
      !contains(stress, "revision <= 128") ||
      !contains(stress, "cycle < 64")) return 2;
  const char* targets[]{
      "win32_uia_provider_object_test", "win32_uia_tree_navigation_test",
      "win32_uia_get_object_message_test", "win32_uia_pattern_provider_test",
      "win32_uia_live_event_test", "win32_uia_runtime_change_integration_test",
      "win32_uia_provider_lifetime_test", "win32_uia_lifecycle_stress_test",
      "phase_g_win32_uia_provider_object_structure_test",
      "phase_g_win32_uia_tree_navigation_structure_test",
      "phase_g_win32_uia_pattern_provider_structure_test",
      "phase_g_win32_uia_live_event_structure_test",
      "phase_g_win32_uia_change_integration_structure_test",
      "phase_g_win32_uia_provider_lifetime_structure_test",
      "phase_g_win32_uia_lifecycle_stress_structure_test"};
  for (const char* target : targets) {
    if (!contains(xmake, (std::string{"target(\""} + target + "\")").c_str())) {
      return 3;
    }
  }
  if (!contains(source_guard, "win32_uia_provider_state.cpp") ||
      !contains(source_guard, "win32_uia_range_pattern.cpp") ||
      !contains(source_guard, "win32_uia_tree_state.cpp") ||
      !contains(roadmap, "- [x] Steps 619-626: Implement Win32 UIA")) return 4;
  if (!contains(xmake, "target(\"phase_g_win32_uia_closeout_test\")") ||
      !contains(xmake,
                "tests/api_parity/phase_g_win32_uia_closeout_test.cpp")) {
    return 5;
  }

  constexpr const char* completion =
      "Phase G Step 626 audits and closes the Win32 UIA production band, "
      "freezing provider objects, navigation, patterns, live events, runtime "
      "change integration, provider lifetime, diagnostics, and stress "
      "evidence. Step 627 Linux AT-SPI D-Bus object exposure production "
      "behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 6;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 643 animation transition production behavior\"")) return 7;
  return 0;
}
