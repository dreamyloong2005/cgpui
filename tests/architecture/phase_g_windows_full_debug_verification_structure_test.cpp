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
  const std::string phase_c_audit =
      read_source("tests/api_parity/phase_c_final_ledger_audit_test.cpp");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string predecessor = read_source(
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
      &phase_c_audit, &vocabulary, &predecessor, &xmake, &manifest,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (contains(phase_c_audit, "production loading/upload remains later") ||
      contains(phase_c_audit, "production decoding/rendering remains later") ||
      !contains(phase_c_audit, "This closes the public SVG/image") ||
      !contains(phase_c_audit, "front-end authoring band") ||
      !contains(phase_c_audit,
                "while keeping SVG decoding, PNG/JPEG loading, renderer upload,") ||
      !contains(vocabulary, "out of scope for this freeze")) {
    return 2;
  }

  if (!contains(xmake,
                "target(\"phase_g_windows_full_debug_verification_structure_test\")") ||
      !contains(xmake,
                "tests/architecture/phase_g_windows_full_debug_verification_structure_test.cpp") ||
      !contains(manifest,
                "phase_g_windows_full_debug_verification_structure_test|"
                "tests/architecture/phase_g_windows_full_debug_verification_structure_test.cpp")) {
    return 3;
  }
  if (!contains(predecessor, "Step 673 Windows full-debug ") ||
      !contains(predecessor, "verification is next.")) {
    return 4;
  }

  constexpr const char* completion =
      "Phase G Step 673 completes Windows full-debug verification at 450/450 "
      "after updating the Phase C final ledger audit to guard stable historical "
      "SVG/image scope rather than mutable current asset status. Step 674 WSL "
      "full-debug verification is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 5;
  }
  if (!contains(ledger_json, "\"phase_g_step_673_sources\"") ||
      !contains(ledger_json,
                "\"phase_g_step_673_remaining_gap\": \"Step 674 WSL "
                "full-debug verification\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 678 Phase G final closeout "
                "verification\"")) {
    return 6;
  }
  if (line_count(xmake) > 4360 || line_count(manifest) > 160 ||
      line_count(phase_c_audit) > 220) {
    return 7;
  }
  return 0;
}
