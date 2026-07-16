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

std::size_t count(const std::string& text, const char* value) {
  std::size_t result = 0;
  std::size_t offset = 0;
  while ((offset = text.find(value, offset)) != std::string::npos) {
    ++result;
    offset += std::char_traits<char>::length(value);
  }
  return result;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string windows_dependencies =
      read_source("scripts/ci/windows-dependencies.ps1");
  const std::string linux_dependencies =
      read_source("scripts/ci/linux-dependencies.sh");
  const std::string action =
      read_source(".github/actions/setup-phase-g-dependencies/action.yml");
  const std::string lock = read_source("xmake-requires.lock");
  const std::string windows_package =
      read_source("scripts/ci/windows-package.ps1");
  const std::string linux_package = read_source("scripts/ci/linux-package.sh");
  const std::string windows_examples =
      read_source("scripts/ci/windows-example-smoke.ps1");
  const std::string linux_examples =
      read_source("scripts/ci/linux-example-smoke.sh");
  const std::string windows_matrix =
      read_source("scripts/ci/windows-architecture-header.ps1");
  const std::string linux_matrix =
      read_source("scripts/ci/linux-architecture-header.sh");
  const std::string workflow =
      read_source(".github/workflows/phase-g-windows-linux.yml");
  const std::string manifest =
      read_source("scripts/ci/architecture-header-targets.txt");
  const std::string previous = read_source(
      "tests/architecture/phase_g_architecture_header_matrix_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &windows_dependencies, &linux_dependencies, &action, &lock,
      &windows_package, &linux_package, &windows_examples, &linux_examples,
      &windows_matrix, &linux_matrix, &workflow, &manifest, &previous, &xmake,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(xmake, "set_policy(\"package.requires_lock\", true)") ||
      !contains(lock, "__meta__") || !contains(lock, "version = \"1.0\"") ||
      !contains(lock, "[\"windows|x64\"]") ||
      !contains(lock, "[\"linux|x86_64\"]") ||
      !contains(lock, "lunasvg") || !contains(lock, "vulkansdk") ||
      !contains(lock, "libxkbcommon")) return 2;
  constexpr const char* canonical_repository =
      "url = \"https://github.com/xmake-io/xmake-repo.git\"";
  if (contains(lock, "https://gitee.com/tboox/xmake-repo.git") ||
      !contains(lock, canonical_repository) ||
      count(lock, "url = ") != count(lock, canonical_repository)) return 2;

  if (!contains(windows_dependencies, "dependencies\\windows") ||
      !contains(windows_dependencies, "XMAKE_GLOBALDIR") ||
      !contains(windows_dependencies, "XMAKE_PKG_CACHEDIR") ||
      !contains(windows_dependencies, "XMAKE_PKG_INSTALLDIR") ||
      !contains(windows_dependencies, "GITHUB_ENV")) return 3;
  if (!contains(linux_dependencies, "dependencies/linux") ||
      !contains(linux_dependencies, "XMAKE_GLOBALDIR") ||
      !contains(linux_dependencies, "XMAKE_PKG_CACHEDIR") ||
      !contains(linux_dependencies, "XMAKE_PKG_INSTALLDIR") ||
      !contains(linux_dependencies, "GITHUB_ENV")) return 4;

  const std::string* windows_consumers[]{
      &windows_package, &windows_examples, &windows_matrix};
  for (const auto* consumer : windows_consumers) {
    if (!contains(*consumer, "windows-dependencies.ps1") ||
        contains(*consumer, "xrepo update-repo")) return 5;
  }
  const std::string* linux_consumers[]{
      &linux_package, &linux_examples, &linux_matrix};
  for (const auto* consumer : linux_consumers) {
    if (!contains(*consumer, "linux-dependencies.sh") ||
        contains(*consumer, "xrepo update-repo")) return 6;
  }

  if (!contains(action, "xmake-version: 3.0.9") ||
      !contains(action, "actions/cache@v4") ||
      !contains(action, "pkg-cache") || !contains(action, "pkg-install") ||
      !contains(action, "windows-dependencies.ps1") ||
      !contains(action, "linux-dependencies.sh") ||
      count(workflow, "uses: ./.github/actions/setup-phase-g-dependencies") != 8 ||
      contains(workflow, "xmake-version: latest")) return 7;

  if (!contains(previous, "Phase G Step 671 adds Windows and Linux CI") ||
      !contains(manifest,
          "phase_g_reproducible_dependencies_structure_test|") ||
      !contains(xmake,
          "target(\"phase_g_reproducible_dependencies_structure_test\")")) {
    return 8;
  }
  if (line_count(windows_dependencies) > 100 ||
      line_count(linux_dependencies) > 100 || line_count(action) > 80 ||
      line_count(workflow) > 155 || line_count(xmake) > 4385) return 9;

  constexpr const char* completion =
      "Phase G Step 672 pins Xmake 3.0.9, commits a dual-platform package "
      "lock, centralizes workspace-confined dependency roots, caches "
      "reproducible package state in CI, and removes floating repository "
      "refreshes from package setup. Step 673 Windows full-debug "
      "verification is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 10;
  }
  if (!contains(ledger_json, "\"phase_g_step_672_sources\"") ||
      !contains(ledger_json,
          "\"phase_f_current_handoff\": \"Phase I Step 759 X11/XCB platform boundary\"")) {
    return 11;
  }
  return 0;
}
