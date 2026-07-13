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
  const std::string adapter_header = read_source(
      "src/platform/win32/win32_accessibility_internal.hpp");
  const std::string adapter = read_source(
      "src/platform/win32/win32_accessibility.cpp");
  const std::string updates = read_source(
      "src/platform/win32/win32_uia_updates.cpp");
  const std::string behavior = read_source(
      "tests/platform/win32_uia_lifecycle_stress_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &lifetime_header, &lifetime, &adapter_header, &adapter, &updates,
      &behavior, &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan,
      &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(lifetime_header, "Win32UiaProviderLifetimeDiagnostics") ||
      !contains(lifetime_header, "created_count") ||
      !contains(lifetime_header, "reused_count") ||
      !contains(lifetime_header, "retired_count") ||
      !contains(lifetime_header, "active_count")) return 2;
  if (!contains(lifetime, "diagnostics.created_count") ||
      !contains(lifetime, "diagnostics.reused_count") ||
      !contains(lifetime, "diagnostics.retired_count") ||
      !contains(lifetime, "diagnostics.active_count")) return 3;
  if (!contains(adapter_header, "last_lifetime_diagnostics() const") ||
      !contains(adapter, "last_lifetime_diagnostics() const") ||
      !contains(updates, "last_lifetime_diagnostics_ =")) return 4;
  if (!contains(behavior, "std::thread reader") ||
      !contains(behavior, "revision <= 128") ||
      !contains(behavior, "cycle < 64") ||
      !contains(behavior, "replacement == retired") ||
      !contains(behavior, "UIA_E_ELEMENTNOTAVAILABLE")) return 5;
  if (!contains(xmake, "target(\"win32_uia_lifecycle_stress_test\")") ||
      !contains(xmake,
                "phase_g_win32_uia_lifecycle_stress_structure_test.cpp")) {
    return 6;
  }
  if (line_count(lifetime_header) > 40 || line_count(lifetime) > 90 ||
      line_count(adapter_header) > 80 || line_count(adapter) > 120 ||
      line_count(updates) > 80 || line_count(behavior) > 180) return 7;

  constexpr const char* completion =
      "Phase G Step 625 verifies Win32 UIA lifecycle diagnostics and stress "
      "behavior across 128 concurrent stable updates and 64 remove/recreate "
      "cycles. Step 626 Win32 UIA production closeout audit is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 661 GPUI-style timer control production behavior\"")) return 9;
  return 0;
}
