#include <array>
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
std::string target_block(const std::string& xmake, const char* target) {
  const std::string marker = std::string{"target(\""} + target + "\")";
  const auto begin = xmake.find(marker);
  if (begin == std::string::npos) return {};
  const auto end = xmake.find("\ntarget(\"", begin + marker.size());
  return xmake.substr(begin, end == std::string::npos ? end : end - begin);
}
} // namespace

int main() {
  const std::string xmake = read_source("xmake.lua");
  const std::string windows_guard = read_source(
      "tests/architecture/phase_f_windows_full_debug_verification_structure_test.cpp");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&xmake, &windows_guard, &roadmap, &ledger_md,
                                &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  constexpr std::array repository_tests{
      "app_source_structure_test",
      "desktop_target_readiness_test",
      "hello_window_lifetime_test",
      "public_api_compatibility_examples_test",
      "public_api_example_expansion_test",
      "public_async_test_workflow_example_test",
      "public_authoring_vocabulary_freeze_test",
      "public_context_capability_example_test",
      "public_phase_b_completion_audit_test",
      "public_phase_b_surface_closure_example_test",
  };
  for (std::size_t index = 0; index < repository_tests.size(); ++index) {
    const std::string block = target_block(xmake, repository_tests[index]);
    if (!contains(block, "rundir = os.projectdir()") ||
        !contains(block, "CGPUI_SOURCE_ROOT = os.projectdir()")) {
      return 10 + static_cast<int>(index);
    }
  }
  if (!contains(xmake,
                "target(\"phase_f_wsl_full_debug_verification_structure_test\")") ||
      !contains(windows_guard,
                "Step 663 GPUI-style rendering control production behavior")) {
    return 20;
  }

  constexpr const char* completion =
      "Phase F Step 612 completes WSL full-debug verification at 321/321 after binding historical repository-inspection targets to the project root, including real WSLg Wayland frame capture. Step 613 cross-platform test execution audit is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 21;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 663 GPUI-style rendering control production behavior\"")) return 22;
  return 0;
}
