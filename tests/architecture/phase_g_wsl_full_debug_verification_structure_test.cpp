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
  const std::string predecessor = read_source(
      "tests/architecture/phase_g_windows_full_debug_verification_structure_test.cpp");
  const std::string dependency_guard = read_source(
      "tests/architecture/phase_g_reproducible_dependency_setup_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string manifest =
      read_source("scripts/ci/architecture-header-targets.txt");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &predecessor, &dependency_guard, &xmake, &manifest, &roadmap,
      &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(xmake,
                "target(\"phase_g_wsl_full_debug_verification_structure_test\")") ||
      !contains(xmake,
                "tests/architecture/phase_g_wsl_full_debug_verification_structure_test.cpp") ||
      !contains(manifest,
                "phase_g_wsl_full_debug_verification_structure_test|"
                "tests/architecture/phase_g_wsl_full_debug_verification_structure_test.cpp")) {
    return 2;
  }
  if (!contains(predecessor, "Step 674 WSL ") ||
      !contains(predecessor, "full-debug verification is next.") ||
      !contains(xmake, "set_policy(\"package.requires_lock\", true)") ||
      !contains(dependency_guard, "3.0.9")) {
    return 3;
  }

  constexpr const char* completion =
      "Phase G Step 674 completes WSL full-debug verification at 431/431 with "
      "real WSLg Wayland frame pixel capture under the locked Xmake 3.0.9 "
      "dependency environment. Step 675 action macro production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 4;
  }
  if (!contains(ledger_json, "\"phase_g_step_674_sources\"") ||
      !contains(ledger_json,
                "\"phase_g_step_674_remaining_gap\": \"Step 675 action macro "
                "production behavior\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Phase H Step 679 Cocoa application "
                "and NSWindow lifecycle\"")) {
    return 5;
  }
  if (line_count(xmake) > 4360 || line_count(manifest) > 165 ||
      line_count(dependency_guard) > 180) {
    return 6;
  }
  return 0;
}
