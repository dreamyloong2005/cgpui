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

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string windows_core = read_source("scripts/ci/windows-package.ps1");
  const std::string windows_debug = read_source("scripts/ci/windows-debug.ps1");
  const std::string windows_release = read_source("scripts/ci/windows-release.ps1");
  const std::string linux_core = read_source("scripts/ci/linux-package.sh");
  const std::string linux_debug = read_source("scripts/ci/linux-debug.sh");
  const std::string linux_release = read_source("scripts/ci/linux-release.sh");
  const std::string workflow =
      read_source(".github/workflows/phase-g-windows-linux.yml");
  const std::string manifests = read_source("tests/architecture/mcpp.toml");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string* required[]{
      &windows_core, &windows_debug, &windows_release, &linux_core,
      &linux_debug, &linux_release, &workflow, &manifests,
      &ledger_json};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(windows_debug, "windows-package.ps1") ||
      !contains(windows_debug, "Debug") ||
      !contains(windows_release, "windows-package.ps1") ||
      !contains(windows_release, "Release") ||
      !contains(windows_core, "ValidateSet(\"Debug\", \"Release\")") ||
      !contains(windows_core, "windows-dependencies.ps1") ||
      !contains(windows_core, "& $mcpp build --profile $profileName") ||
      contains(windows_core, "-j 1")) return 2;

  if (!contains(linux_debug, "linux-package.sh") ||
      !contains(linux_debug, "debug") ||
      !contains(linux_release, "linux-package.sh") ||
      !contains(linux_release, "release") ||
      !contains(linux_core, "debug|release") ||
      !contains(linux_core, "linux-dependencies.sh") ||
      !contains(linux_core, "\"$MCPP\" build --profile \"$profile\"") ||
      contains(linux_core, "-j 1")) return 3;

  const char* artifacts[]{
      "cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui",
      "cgpui_app", "hello_window", "include/cgpui", "README.md",
      "manifest.json"};
  for (const char* artifact : artifacts) {
    if (!contains(windows_core, artifact) || !contains(linux_core, artifact)) {
      return 4;
    }
  }

  if (!contains(workflow, "windows-release:") ||
      !contains(workflow, "scripts/ci/windows-release.ps1") ||
      !contains(workflow, "build/phase-g-ci/windows-release/package") ||
      !contains(workflow, "linux-release:") ||
      !contains(workflow, "scripts/ci/linux-release.sh") ||
      !contains(workflow, "build/phase-g-ci/linux-release/package") ||
      !contains(workflow, "uses: ./.github/actions/setup-phase-g-dependencies") ||
      !contains(workflow, "cgpui-windows-release") ||
      !contains(workflow, "cgpui-linux-release")) return 5;

  if (!contains(manifests,
                "\"phase_g_release_packaging_structure_test.cpp\"")) {
    return 6;
  }
  if (line_count(windows_core) > 190 || line_count(linux_core) > 190 ||
      line_count(windows_debug) > 35 || line_count(windows_release) > 35 ||
      line_count(linux_debug) > 20 || line_count(linux_release) > 20 ||
      line_count(workflow) > 150 || line_count(read_source("mcpp.toml")) > 100) return 7;

  if (!contains(ledger_json, "\"phase_g_step_669_sources\"")) {
    return 9;
  }
  return 0;
}
