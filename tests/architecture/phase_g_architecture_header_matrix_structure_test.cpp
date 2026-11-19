#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <unordered_set>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const std::string& value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  namespace fs = std::filesystem;
  const char* root_value = CGPUI_SOURCE_ROOT;
  const fs::path root = root_value == nullptr ? fs::path{"."} : root_value;
  const std::string matrix =
      read_source("scripts/ci/architecture-header-targets.txt");
  const std::string windows =
      read_source("scripts/ci/windows-architecture-header.ps1");
  const std::string linux =
      read_source("scripts/ci/linux-architecture-header.sh");
  const std::string workflow =
      read_source(".github/workflows/phase-g-windows-linux.yml");
  const std::string previous = read_source(
      "tests/architecture/phase_g_example_smoke_matrix_structure_test.cpp");
  // Each matrix entry names a test program of tests/architecture or
  // tests/header_cleanliness; mcpp names it after its file.
  const std::string manifests = read_source("tests/architecture/mcpp.toml") +
      read_source("tests/header_cleanliness/mcpp.toml");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &matrix, &windows, &linux, &workflow, &previous, &manifests,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  const std::string& registered_targets = manifests;

  std::unordered_set<std::string> targets;
  std::unordered_set<std::string> sources;
  std::size_t offset = 0;
  while (offset < matrix.size()) {
    const std::size_t end = matrix.find('\n', offset);
    std::string line = matrix.substr(offset, end - offset);
    offset = end == std::string::npos ? matrix.size() : end + 1;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line.empty() || line.front() == '#') continue;
    const std::size_t separator = line.find('|');
    if (separator == std::string::npos || line.find('|', separator + 1) != std::string::npos) return 2;
    const std::string target = line.substr(0, separator);
    const std::string source = line.substr(separator + 1);
    if (target.empty() || source.empty() || !targets.insert(target).second ||
        !sources.insert(source).second) return 2;
    if (!source.starts_with("tests/architecture/") &&
        !source.starts_with("tests/header_cleanliness/")) return 2;
    const std::string file = source.substr(source.rfind('/') + 1);
    if (!fs::is_regular_file(root / source) || file != target + ".cpp" ||
        !contains(registered_targets, "\"" + file + "\"")) {
      return 3;
    }
  }

  std::size_t repository_sources = 0;
  for (const char* directory : {"tests/architecture", "tests/header_cleanliness"}) {
    for (const auto& entry : fs::directory_iterator(root / directory)) {
      if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
        ++repository_sources;
      }
    }
  }
  if (targets.size() < 142 || sources.size() != repository_sources) return 4;

  if (!contains(windows, "windows-package.ps1") ||
      !contains(windows, "ReusePreparedRoot") ||
      !contains(windows, "architecture-header-targets.txt") ||
      !contains(windows, "& $mcpp test --list") || contains(windows, "-j 1") ||
      !contains(windows, "& $mcpp test -p tests/architecture")) return 5;
  if (!contains(linux, "linux-package.sh") ||
      !contains(linux, "CGPUI_CI_REUSE_PREPARED_ROOT") ||
      !contains(linux, "linux-dependencies.sh") ||
      !contains(linux, "architecture-header-targets.txt") ||
      !contains(linux, "\"$MCPP\" test --list") || contains(linux, "-j 1") ||
      !contains(linux, "-p tests/header_cleanliness")) return 6;
  if (!contains(workflow, "windows-architecture-header:") ||
      !contains(workflow, "scripts/ci/windows-architecture-header.ps1") ||
      !contains(workflow, "linux-architecture-header:") ||
      !contains(workflow, "scripts/ci/linux-architecture-header.sh")) return 7;
  if (!contains(previous, "Phase G Step 670 adds Windows and Linux CI") ||
      !contains(manifests,
                "\"phase_g_architecture_header_matrix_structure_test.cpp\"")) return 8;
  if (line_count(windows) > 105 ||
      line_count(linux) > 95 || line_count(workflow) > 170 ||
      line_count(read_source("mcpp.toml")) > 100) return 9;

  constexpr const char* completion =
      "Phase G Step 671 adds Windows and Linux CI architecture/header "
      "matrices that serially build every registered architecture and "
      "header-cleanliness target, execute all 142 tests from a shared "
      "source-owned manifest, and fail closed on missing, duplicate, "
      "unmapped, or unregistered coverage. Step 672 reproducible dependency "
      "setup is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 10;
  }
  if (!contains(ledger_json, "\"phase_g_step_671_sources\"") ||
      !contains(ledger_json,
          "\"phase_f_current_handoff\": \"Phase J Step 799 re-run upstream extractor against the pinned revision\"")) {
    return 11;
  }
  return 0;
}
