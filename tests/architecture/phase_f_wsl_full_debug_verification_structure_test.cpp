#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}
bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}
bool registered_with_source_root(const char* test) {
  // tests/<suite>/<file>.cpp: listed in the suite's [test] discover, whose
  // build.mcpp defines CGPUI_SOURCE_ROOT, and read through that define.
  const std::string path{test};
  const auto slash = path.rfind('/');
  const std::string suite = path.substr(0, slash);
  const std::string file = path.substr(slash + 1);
  return contains(read_source((suite + "/mcpp.toml").c_str()), ("\"" + file + "\"").c_str()) &&
      contains(read_source((suite + "/build.mcpp").c_str()), "source_root_define(2)") &&
      contains(read_source(path.c_str()), "CGPUI_SOURCE_ROOT");
}
} // namespace

int main() {
  const std::string manifests = read_source("tests/architecture/mcpp.toml");
  const std::string windows_guard = read_source(
      "tests/architecture/phase_f_windows_full_debug_verification_structure_test.cpp");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&manifests, &windows_guard, &roadmap, &ledger_md,
                                &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  constexpr std::array repository_tests{
      "tests/architecture/app_source_structure_test.cpp",
      "tests/architecture/desktop_target_readiness_test.cpp",
      "tests/architecture/hello_window_lifetime_test.cpp",
      "tests/api_parity/public_api_compatibility_examples_test.cpp",
      "tests/api_parity/public_api_example_expansion_test.cpp",
      "tests/api_parity/public_async_test_workflow_example_test.cpp",
      "tests/api_parity/public_authoring_vocabulary_freeze_test.cpp",
      "tests/api_parity/public_context_capability_example_test.cpp",
      "tests/api_parity/public_phase_b_completion_audit_test.cpp",
      "tests/api_parity/public_phase_b_surface_closure_example_test.cpp",
  };
  for (std::size_t index = 0; index < repository_tests.size(); ++index) {
    if (!registered_with_source_root(repository_tests[index])) {
      return 10 + static_cast<int>(index);
    }
  }
  if (!contains(manifests,
                "\"phase_f_wsl_full_debug_verification_structure_test.cpp\"") ||
      !contains(windows_guard,
                "Phase J Step 799 re-run upstream extractor against the pinned revision")) {
    return 20;
  }

  constexpr const char* completion =
      "Phase F Step 612 completes WSL full-debug verification at 321/321 after binding historical repository-inspection targets to the project root, including real WSLg Wayland frame capture. Step 613 cross-platform test execution audit is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 21;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Phase J Step 799 re-run upstream extractor against the pinned revision\"")) return 22;
  return 0;
}
