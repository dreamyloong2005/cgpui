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
  const std::string lifetime_header = read_source(
      "src/platform/win32/win32_uia_lifetime_internal.hpp");
  const std::string lifetime = read_source(
      "src/platform/win32/win32_uia_lifetime.cpp");
  const std::string provider_object = read_source(
      "src/platform/win32/win32_uia_provider_object_internal.hpp");
  const std::string provider_state = read_source(
      "src/platform/win32/win32_uia_provider_state.cpp");
  const std::string tree_state_header = read_source(
      "src/platform/win32/win32_uia_tree_state_internal.hpp");
  const std::string tree_state = read_source(
      "src/platform/win32/win32_uia_tree_state.cpp");
  const std::string updates = read_source(
      "src/platform/win32/win32_uia_updates.cpp");
  const std::string behavior = read_source(
      "tests/platform/win32_uia_provider_lifetime_test.cpp");
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
      &lifetime_header, &lifetime, &provider_object, &provider_state,
      &tree_state_header, &tree_state, &updates, &behavior, &source_guard,
      &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(lifetime, "reconcile_win32_uia_provider_lifetime(") ||
      !contains(lifetime, "replace_win32_uia_provider_node(") ||
      !contains(lifetime, "retire_win32_uia_provider(") ||
      !contains(updates, "reconcile_win32_uia_provider_lifetime(")) return 2;
  if (!contains(provider_object, "std::mutex state_mutex_") ||
      !contains(provider_object, "bool available_") ||
      !contains(provider_state, "Win32UiaProvider::snapshot() const") ||
      !contains(provider_state, "Win32UiaProvider::retire()") ||
      !contains(provider_state, "UIA_E_ELEMENTNOTAVAILABLE")) return 3;
  if (!contains(tree_state_header, "class Win32UiaProviderTree") ||
      !contains(tree_state, "replace_win32_uia_provider_tree_state(") ||
      !contains(tree_state, "invalidate_win32_uia_provider_tree(")) return 4;
  if (!contains(behavior, "adapter.provider_for_element(2) != input") ||
      !contains(behavior, "UIA_E_ELEMENTNOTAVAILABLE") ||
      !contains(behavior, "replacement == input") ||
      !contains(behavior, "retained_after_destroy")) return 5;
  if (!contains(source_guard, "win32_uia_lifetime.cpp") ||
      !contains(source_guard, "win32_uia_provider_state.cpp") ||
      !contains(source_guard, "win32_uia_tree_state.cpp") ||
      !contains(xmake, "target(\"win32_uia_provider_lifetime_test\")") ||
      !contains(xmake,
                "phase_g_win32_uia_provider_lifetime_structure_test.cpp")) {
    return 6;
  }
  if (line_count(lifetime_header) > 40 || line_count(lifetime) > 90 ||
      line_count(provider_object) > 90 || line_count(provider_state) > 80 ||
      line_count(tree_state_header) > 70 || line_count(tree_state) > 70 ||
      line_count(updates) > 80 || line_count(behavior) > 140) return 7;

  constexpr const char* completion =
      "Phase G Step 624 preserves Win32 UIA provider COM identity across "
      "stable element updates, refreshes provider state in place, and retires "
      "removed or destroyed providers with element-unavailable semantics. "
      "Step 625 Win32 UIA lifecycle stress production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 666 GPUI-style test support closeout audit\"")) return 9;
  return 0;
}
