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

}  // namespace

int main() {
  const std::string script = read_source("scripts/ci/macos-debug.sh");
  const std::string manifests = read_source("third_party/VENDORED.md");
  const std::string dependencies =
      read_source("scripts/ci/macos-dependencies.sh");
  const std::string targets = read_source("tests/architecture/mcpp.toml");
  const std::string manifest = read_source("scripts/ci/architecture-header-targets.txt");
  const std::string workflow = read_source(".github/workflows/phase-h-macos.yml");
  const std::string task_plan = read_source("task_plan.md");
  const std::string progress = read_source("progress.md");
  const std::string findings = read_source("findings.md");
  if (script.empty() || manifests.empty() || dependencies.empty() ||
      targets.empty() || manifest.empty() || workflow.empty()) {
    return 1;
  }
  const std::size_t confinement = script.find("case \"$output_root/\"");
  const std::size_t traversal_rejection =
      script.find("output root must not contain parent traversal");
  const std::size_t output_creation = script.find("mkdir -p \"$output_root\"");
  if (!contains(script, "set -euo pipefail") ||
      !contains(script, "cgpui_require_macos_arm64") ||
      !contains(script, "output root must not contain parent traversal") ||
      !contains(script, "output root must not be the repository root") ||
      !contains(script, "mcpp 2026.11.20.1 or later") ||
      !contains(script, "source \"$repo_root/scripts/ci/macos-dependencies.sh\"") ||
      !contains(script, "cgpui_configure_macos_dependency_environment") ||
      !contains(script, "MACOSX_DEPLOYMENT_TARGET") ||
      !contains(dependencies, "MCPP_HOME") ||
      !contains(script, "build/phase-h-ci/macos-debug") ||
      !contains(script, "sw_vers -productVersion") ||
      !contains(script, "xcodebuild -version") ||
      !contains(script, "\"$mcpp\" build") ||
      !contains(script, "\"$mcpp\" test") ||
      !contains(script, "git diff --check") ||
      confinement == std::string::npos ||
      traversal_rejection == std::string::npos ||
      output_creation == std::string::npos ||
      traversal_rejection >= output_creation || confinement >= output_creation) {
    return 2;
  }
  if (!contains(manifests, "Source archive SHA-256") ||
      !contains(targets,
                "\"phase_h_macos_full_debug_verification_structure_test.cpp\"") ||
      !contains(manifest,
                "phase_h_macos_full_debug_verification_structure_test|") ||
      !contains(workflow, "macos-full-debug:") ||
      !contains(workflow, "scripts/ci/macos-debug.sh")) {
    return 3;
  }
  constexpr const char* evidence =
      "Phase H macOS full debug passes 380/380 on macOS 26.5.2 (25F84), "
      "Xcode 26.6 (17F113), and Xmake 3.0.9+HEAD.2b184e178, including "
      "native Cocoa, Metal primitive/clip/text-image pixel capture, "
      "accessibility, and public-example smoke coverage.";
  if (!contains(task_plan, evidence) || !contains(progress, evidence) ||
      !contains(findings, evidence)) {
    return 4;
  }
  return 0;
}
