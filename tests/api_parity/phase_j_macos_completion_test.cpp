#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, std::string_view value) {
  return text.find(value) != std::string::npos;
}

std::size_t count(const std::string& text, std::string_view value) {
  std::size_t result = 0;
  for (std::size_t offset = text.find(value); offset != std::string::npos;
       offset = text.find(value, offset + value.size())) {
    ++result;
  }
  return result;
}

}  // namespace

int main() {
  const std::string dependencies =
      read_source("scripts/ci/macos-dependencies.sh");
  const std::string matrix =
      read_source("docs/gpui-official-example-matrix.json");
  const std::string example_audit =
      read_source("docs/gpui-phase-j-example-audit.json");
  const std::string performance =
      read_source("docs/gpui-performance-baseline-policy.json");
  const std::string stress = read_source("docs/gpui-stress-profile.json");
  const std::string release =
      read_source("docs/gpui-release-artifact-policy.json");
  const std::string packer =
      read_source("scripts/ci/create-release-artifact.py");
  const std::string package = read_source("scripts/ci/macos-package.sh");
  const std::string release_script =
      read_source("scripts/ci/macos-phase-j-release.sh");
  const std::string performance_script =
      read_source("scripts/ci/macos-performance-baseline.sh");
  const std::string stress_script =
      read_source("scripts/ci/macos-stress-matrix.sh");
  const std::string architecture_script =
      read_source("scripts/ci/macos-architecture-header.sh");
  const std::string workflow =
      read_source(".github/workflows/phase-j-desktop.yml");
  const std::string verification =
      read_source("docs/gpui-phase-j-final-verification.json");
  const std::string ledger =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");

  if (!contains(dependencies, "cgpui_configure_macos_dependency_environment") ||
      !contains(dependencies, "b9256335e0b6e70808e23dfe71627d8a4dcc0abf") ||
      !contains(dependencies, "--github-env")) {
    return 10;
  }
  if (count(matrix, "\"macos\": \"compiled_and_smoked\"") != 20 ||
      !contains(example_audit, "\"macos_execution\": \"required\"")) {
    return 20;
  }
  if (!contains(performance, "\"macos-arm64\"") ||
      !contains(performance, "\"macos-x86_64\"") ||
      performance_script.empty()) {
    return 30;
  }
  if (!contains(stress, "\"macos-arm64\"") ||
      !contains(stress, "\"macos-x86_64\"") || stress_script.empty()) {
    return 40;
  }
  if (package.empty() || release_script.empty() ||
      !contains(release, "cgpui-macos-arm64-release.tar.gz") ||
      !contains(release, "cgpui-macos-x86_64-release.tar.gz") ||
      !contains(packer, "\"macos\"")) {
    return 50;
  }
  if (workflow.empty() || architecture_script.empty() ||
      !contains(workflow, "macos-15-intel") ||
      !contains(workflow, "MACOSX_DEPLOYMENT_TARGET: \"13.0\"")) {
    return 60;
  }
  const bool pending = contains(
      ledger, "\"phase_j_macos_completion_status\": "
              "\"pending_native_dual_arch_verification\"");
  const bool complete =
      contains(ledger, "\"phase_j_status\": \"complete\"") &&
      contains(verification, "\"scope\": \"all_desktop\"") &&
      contains(verification, "\"status\": \"passed\"");
  if ((!pending && !complete) ||
      !contains(ledger_md, "Phase J macOS completion") ||
      !contains(verification, "\"arm64\"") ||
      !contains(verification, "\"x86_64\"")) {
    return 70;
  }
  return 0;
}
